#include "pipeline.h"
#include "command.h"
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


/* ========== Helper functions for the pipeline object ========== */


Pipeline *
get_pipeline_obj(void)
{
    Pipeline *pipeline = malloc(sizeof(*pipeline));
    if (pipeline == NULL) {
        perror("get_pipeline_obj");
        return NULL;
    }

    pipeline->command       = NULL;
    pipeline->gid           = 0;
    pipeline->command_count = 0;
    pipeline->capacity      = 0;
    return pipeline;
}


void
destroy_pipeline_obj(Pipeline *pipeline)
{
    for (int i = 0; i < pipeline->command_count; i++) {
        /* Destroy each command in the pipeline */
        destroy_command_obj(pipeline->command[i]);
    }

    free(pipeline->command);
    free(pipeline);
}


int
add_command_to_pipeline(Pipeline *pipeline, Command *command)
{
    #define INCR_SIZE 1

    if (pipeline->capacity <= pipeline->command_count) {
        pipeline->capacity += INCR_SIZE;

        Command **temp = realloc(pipeline->command, sizeof(*temp) * (pipeline->capacity));
        if (temp == NULL) {
            pipeline->capacity -= 1; /* reset capacity */
            perror("add_command_to_pipeline");
            return -1;
        }

        pipeline->command = temp;
    }

    /* Store the command obj in array */
    pipeline->command[pipeline->command_count] = command;
    pipeline->command_count += 1;

    /* Return index where the command was stored */
    return pipeline->command_count - 1;

    #undef INCR_SIZE
}


/* ========== Pipeline execution and handling functions ========== */


static void
terminate_pipeline(Pipeline *pipeline)
{
    // TODO: Logic for terminating all processes in the pipeline
}


static void
handle_pipeline_suspension(Pipeline *pipeline)
{
    /* Make sure every process in the group is
       suspended, even if only one process was
       supposed to. */
       kill(-pipeline->gid, SIGTSTP);

       Job *job = add_pipeline_to_job(pipeline);
       if (job != NULL) {
            perror("Creating job out of pipeline failed");
            /* Restart the pipeline */
            printf("Restarting the pipeline: GID: %d\n", pipeline->gid);
            kill(-pipeline->gid, SIGCONT);
        }
        // TODO: Notify user about job stopped and put in background
}


static int
wait_for_pipeline(Pipeline *pipeline)
{
    siginfo_t infop;

    for (int i = 0; i < pipeline->command_count; i++) {
        if (waitid(P_PGID, pipeline->gid, &infop, WEXITED | WSTOPPED) == -1) {
            perror("Waiting for child failed");
            return -1;
        }

        /* TO know whether the process was stopped or finished */
        int si_code = infop.si_code;

        switch (si_code) {
            case CLD_STOPPED:
                handle_pipeline_suspension(pipeline);
                break;

            case CLD_EXITED: {  /* if the command terminated */
                pid_t child_pid    = infop.si_pid;
                int   child_status = infop.si_status;

                /* Find the command with specified pid and update its status */
                for (int i = 0; i < pipeline->command_count; i++) {
                    if (pipeline->command[i]->pid == child_pid) {
                        pipeline->command[i]->return_status = child_status;
                        // TODO: Make a better command updater
                    }
                }
            }
        }

    }

    return 0;
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
            launch_command(pipeline->command[index], infile, outfile);

        default:    /* parent process */
            /* Update child's process group */
            if (pipeline->gid == 0) {
                pipeline->gid = pid;
            }
            setpgid(pid, pipeline->gid);
            pipeline->command[index]->pid = pid;
            // TODO: Add and update various parameters in command struct
    }

    return 0;
}


#define WRITE_END 1
#define READ_END  0

#define CLEAN_UP_FDS(infile, outfile, pipefd) \
        if (infile != STDIN_FILENO) {         \
            close(infile);                    \
        }                                     \
        if (outfile != STDOUT_FILENO) {       \
            close(outfile);                   \
        }                                     \
        infile = pipefd[0];                   \

int
launch_pipeline(Pipeline *pipeline)
{
    int pipefd[2];
    int infile = STDIN_FILENO;
    int outfile;

    for (int i = 0; i < pipeline->command_count; i++) {
        if (i + 1 == pipeline->command_count) {
            /* For last command in pipeline, connect outfile to stdout */
            outfile = STDOUT_FILENO;
        } else {
            if (pipe(pipefd) < 0) {
                perror("Pipe creation failed");
                // TODO: Terminate all commands in pipeline
                return -1;
            }
            outfile = pipefd[WRITE_END];
        }

        if (create_and_exec_child_process(pipeline, i, infile, outfile) == -1) {
            // TODO: Terminate all running commands in pipeline
            return -1;
        }

        CLEAN_UP_FDS(infile, outfile, pipefd);
    }

    /* Put the pipeline as the foreground process (group) */
    tcsetpgrp(get_shell_terminal(), pipeline->gid);
    wait_for_pipeline(pipeline);
    put_shell_in_foreground();

    /* Return status of last command in pipeline */
    return pipeline->command[pipeline->command_count - 1]->return_status;
}

#undef WRITE_END
#undef READ_END
#undef CLEAN_UP_FDS