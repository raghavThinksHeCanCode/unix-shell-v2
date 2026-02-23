#include "pipeline.h"
#include "process.h"
#include "shell.h"
#include "job.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


static void terminate_pipeline(Pipeline *pipeline);
static int handle_pipeline_suspension(Pipeline *pipeline, bool in_subshell);
static void handle_pipeline_termination(Pipeline *pipeline, int sig);
static Pipe_return_status wait_for_pipeline(Pipeline *pipeline, bool in_subshell);
static int create_and_exec_child_process(Pipeline *pipeline, int index, int infile, int outfile, bool in_subshell);
static int setup_and_launch_pipeline(Pipeline *pipeline, bool in_subshell);



/* ========== Helper functions for the pipeline object ========== */


Pipeline *
get_pipeline_obj(void)
{
    Pipeline *pipeline = malloc(sizeof(*pipeline));
    if (pipeline == NULL) {
        perror("get_pipeline_obj");
        return NULL;
    }

    pipeline->process       = NULL;
    pipeline->gid           = 0;
    pipeline->process_count = 0;
    pipeline->capacity      = 0;
    pipeline->is_running    = false;
    return pipeline;
}


void
destroy_pipeline_obj(Pipeline *pipeline)
{
    for (int i = 0; i < pipeline->process_count; i++) {
        /* Destroy each process in the pipeline, only
           if pipeline is not managed by job handler */
        if (pipeline->is_running) {
            destroy_process_obj(pipeline->process[i]);
        }
    }

    free(pipeline->process);
    free(pipeline);
}


int
add_process_to_pipeline(Pipeline *pipeline, Process *process)
{
    #define INCR_SIZE 1

    if (pipeline->capacity <= pipeline->process_count) {
        pipeline->capacity += INCR_SIZE;

        Process **temp = realloc(pipeline->process, sizeof(*temp) * (pipeline->capacity));
        if (temp == NULL) {
            pipeline->capacity -= 1; /* reset capacity */
            perror("add_Process_to_pipeline");
            return -1;
        }

        pipeline->process = temp;
    }

    /* Store the Process obj in array */
    pipeline->process[pipeline->process_count] = process;
    pipeline->process_count += 1;

    /* Return index where the Process was stored */
    return pipeline->process_count - 1;

    #undef INCR_SIZE
}


/* ========== Pipeline execution and handling functions ========== */


static void
terminate_pipeline(Pipeline *pipeline)
{
    // TODO: Logic for terminating all processes in the pipeline
}


static void
handle_pipeline_termination(Pipeline *pipeline, int sig)
{
    /* Make sure every process in group is terminated
       including the subshell, if any. */
    kill(-pipeline->gid, sig);

    pipeline->is_running = false;
}


/* Suspends a pipeline. On success, returns 0. On failure
    returns -1, and continues every process in pipeline. */
static int
handle_pipeline_suspension(Pipeline *pipeline, bool in_subshell)
{
    /* Make sure every process in the group
       is suspended, including the subshell, if any.*/
    kill(-pipeline->gid, SIGTSTP);

    /* If pipeline is suspended and there's no subshell
       involved, this means create a job out of that pipeline */
    if (!in_subshell) {
        Job *job = add_pipeline_to_job(pipeline, true, false);
        if (job == NULL) {
            /* Restart the pipeline */
            printf("shell: Restarting the pipeline: GID: %d\n", pipeline->gid);
            kill(-pipeline->gid, SIGCONT);
            return -1;
        }

        notify_job_status(job);
    }
    return 0;
}


static Pipe_return_status
wait_for_pipeline(Pipeline *pipeline, bool in_subshell)
{
    siginfo_t infop;

    for (int i = 0; i < pipeline->process_count; i++) {
        if (waitid(P_PGID, pipeline->gid, &infop, WEXITED | WSTOPPED) == -1) {
            perror("Waiting for child failed");
            return -1;
        }

        /* TO know whether the process was stopped, finished or terminated */
        int si_code = infop.si_code;

        switch (si_code) {
            case CLD_STOPPED:  /* if process was stopped or suspended */
                if (handle_pipeline_suspension(pipeline, in_subshell) == 0) {
                    /* If pipeline is suspended in subshell,
                       the subshell should continue waiting for 
                       child processes after continuing. If not in subshell,
                       the suspend status must be reported back. */
                    if(!in_subshell) {
                        return PIPE_SUSPND;
                    }
                    break;
                }

            case CLD_KILLED: {   /* if process was terminated/killed */
                /* If pipeline is terminated in subshell, the whole
                   subshell should be stopped. This means the subshell
                   won't be able to execute any more. If not in subshell,
                   only the pipeline that recieved termination signal
                   should be stopped.
                */
                int sig = infop.si_status;
                handle_pipeline_termination(pipeline, sig);
                return PIPE_TERM;
            }

            case CLD_EXITED: {  /* if the Process exited */
                pid_t child_pid    = infop.si_pid;
                int   child_status = infop.si_status;

                /* Find the Process with specified pid and update its status */
                for (int i = 0; i < pipeline->process_count; i++) {
                    if (pipeline->process[i]->pid == child_pid) {
                        pipeline->process[i]->return_val = child_status;
                        // TODO: Make a better Process updater
                    }
                }
            }
        }

    }

    pipeline->is_running = false;
    return PIPE_EXIT;
}


static int
create_and_exec_child_process(Pipeline *pipeline, int index, int infile, int outfile, bool in_subshell)
{
    pid_t pid = fork();

    switch (pid) {
        case -1:  /* fork fails */
            perror("Forking child process failed");
            return -1;

        /* Both parent and child processes need to update the process
           group of the child in order to eliminate race conditions. 
           This should not be done if child was spawned by subshell
           as both child and subshell should remain in same group. */

        case 0:     /* child process */
            if (!in_subshell) {
                pid = getpid();
                pid_t pgid = pipeline->gid;
                if (pgid == 0) {
                    /* If first member of group, make group leader */
                    pgid = pid;
                }
                setpgid(pid, pgid);
            }

            launch_process(pipeline->process[index], infile, outfile);

        default:    /* parent process */
            if (!in_subshell) {
                /* Update child's process group */
                if (pipeline->gid == 0) {
                    pipeline->gid = pid;
                }
                setpgid(pid, pipeline->gid);
            }
            pipeline->process[index]->pid  = pid;
            pipeline->process[index]->pgid = pipeline->gid;
            // TODO: Add and update various parameters in Process struct
    }

    return 0;
}


#define WRITE_END 1
#define READ_END  0

#define CLEAN_UP_FDS(infile, outfile, pipefd)     \
        do {                                      \
            if (infile != STDIN_FILENO) {         \
                close(infile);                    \
            }                                     \
            if (outfile != STDOUT_FILENO) {       \
                close(outfile);                   \
            }                                     \
            infile = pipefd[READ_END];            \
        } while (false);                          \

static int
setup_and_launch_pipeline(Pipeline *pipeline, bool in_subshell)
{
    int pipefd[2];
    int infile = STDIN_FILENO;
    int outfile;

    /* Connect processes to pipeline */
    for (int i = 0; i < pipeline->process_count; i++) {
        if (i + 1 == pipeline->process_count) {
            /* For last Process in pipeline, connect outfile to stdout */
            outfile = STDOUT_FILENO;
        } else {
            if (pipe(pipefd) < 0) {
                perror("Pipe creation failed");
                // TODO: Terminate all Processs in pipeline
                return -1;
            }
            outfile = pipefd[WRITE_END];
        }

        /* Launch the process */
        if (create_and_exec_child_process(pipeline, i, infile, outfile, in_subshell) == -1) {
            // TODO: Terminate all running Processs in pipeline
            return -1;
        }

        CLEAN_UP_FDS(infile, outfile, pipefd);
    }

    pipeline->is_running = true;
    return 0;
}

#undef WRITE_END
#undef READ_END
#undef CLEAN_UP_FDS


Pipe_return_status
launch_pipeline(Pipeline *pipeline, int *return_val, bool in_foreground, bool in_subshell)
{
    if (setup_and_launch_pipeline(pipeline, in_subshell) == -1) {
        return -1;
    }

    if (in_foreground) {
        /* Put the pipeline as the foreground process (group) */
        tcsetpgrp(get_shell_terminal(), pipeline->gid);
    }

    /* Based on return status of waiting, set return values */
    Pipe_return_status return_stat = wait_for_pipeline(pipeline, in_subshell);
    switch (return_stat) {
        case PIPE_EXIT:  /* if pipeline exited successfully, return val of last process in pipeline */
            *return_val = pipeline->process[pipeline->process_count - 1]->return_val;
            break;

        /* The below two won't run if we're in a subshell */
        case PIPE_SUSPND: /* if pipeline suspended, return value is 0 (just like in bash) */
            *return_val = 0;
            break;

        case PIPE_TERM:  /* if pipeline was terminted, its reported as a failure */
            *return_val = 1;
            break;
    }

    if (in_foreground) {
        put_shell_in_foreground();
    }
    return return_stat;
}