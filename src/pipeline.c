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
static int handle_pipeline_suspension(Pipeline *pipeline);
static void handle_pipeline_termination(Pipeline *pipeline, int sig);
static Pipe_return_status wait_for_pipeline(Pipeline *pipeline);
static int create_and_exec_child_process(Pipeline *pipeline, int index, int infile, int outfile);



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
    return pipeline;
}


void
destroy_pipeline_obj(Pipeline *pipeline)
{
    for (int i = 0; i < pipeline->process_count; i++) {
        /* Destroy each Process in the pipeline */
        destroy_process_obj(pipeline->process[i]);
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
    /* Make sure every process in pipeline is terminated */
    kill(-pipeline->gid, sig);

    // TODO: Update running status of each process in pipeline
}


/* Suspends a pipeline. On success, returns 0. On failure
    returns -1, and continues every process in pipeline. */
static int
handle_pipeline_suspension(Pipeline *pipeline)
{
    /* Make sure every process in the group is
       suspended, even if only one process was
       supposed to. */
    kill(-pipeline->gid, SIGTSTP);

    Job *job = add_pipeline_to_job(pipeline, true, false);
    if (job == NULL) {
        /* Restart the pipeline */
        printf("shell: Restarting the pipeline: GID: %d\n", pipeline->gid);
        kill(-pipeline->gid, SIGCONT);
        return -1;
    }

    //TODO: Update every process in pipeline to stopped
    notify_job_status(job);
    return 0;
}


static Pipe_return_status
wait_for_pipeline(Pipeline *pipeline)
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
                if (handle_pipeline_suspension(pipeline) == 0) {
                    /* If pipeline suspension is successful, we can exit waiting */
                    return PIPE_SUSPND;
                }

            case CLD_KILLED: {   /* if process was terminated/killed */
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

    return PIPE_EXIT;
}


static int
create_and_exec_child_process(Pipeline *pipeline, int index, int infile, int outfile)
{
    pid_t pid = fork();

    switch (pid) {
        case -1:  /* fork fails */
            perror("Forking child process failed");
            return -1;

        /* Both parent and child processes need to update the process
           group of the child in order to eliminate race conditions. */

        case 0:     /* child process */
            pid = getpid();
            pid_t pgid = pipeline->gid;
            if (pgid == 0) {
                /* If first member of group, make group leader */
                pgid = pid;
            }
            setpgid(pid, pgid);
            launch_process(pipeline->process[index], infile, outfile);

        default:    /* parent process */
            /* Update child's process group */
            if (pipeline->gid == 0) {
                pipeline->gid = pid;
            }
            setpgid(pid, pipeline->gid);
            pipeline->process[index]->pid = pid;
            // TODO: Add and update various parameters in Process struct
    }

    return 0;
}


#define WRITE_END 1
#define READ_END  0

#define CLEAN_UP_FDS(infile, outfile, pipefd) \
        do {                                      \
            if (infile != STDIN_FILENO) {         \
                close(infile);                    \
            }                                     \
            if (outfile != STDOUT_FILENO) {       \
                close(outfile);                   \
            }                                     \
            infile = pipefd[0];                   \
        } while (false);                          \

Pipe_return_status
launch_pipeline(Pipeline *pipeline, int *return_val)
{
    int pipefd[2];
    int infile = STDIN_FILENO;
    int outfile;

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

        if (create_and_exec_child_process(pipeline, i, infile, outfile) == -1) {
            // TODO: Terminate all running Processs in pipeline
            return -1;
        }

        CLEAN_UP_FDS(infile, outfile, pipefd);
    }

    /* Put the pipeline as the foreground process (group) */
    tcsetpgrp(get_shell_terminal(), pipeline->gid);

    /* Based on return status of waiting, set return values */
    Pipe_return_status return_stat = wait_for_pipeline(pipeline);
    switch (return_stat) {
        case PIPE_EXIT:  /* if pipeline exited successfully, return val of last process in pipeline */
            *return_val = pipeline->process[pipeline->process_count - 1]->return_val;
            break;

        case PIPE_SUSPND: /* if pipeline suspended, return value is 0 (just like in bash) */
            *return_val = 0;
            break;

        case PIPE_TERM:  /* if pipeline was terminted, its reported as a failure */
            *return_val = 1;
            break;
    }

    put_shell_in_foreground();
    return return_stat;
}

#undef WRITE_END
#undef READ_END
#undef CLEAN_UP_FDS