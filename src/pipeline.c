#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>

#include "pipeline.h"
#include "command.h"
#include "shell.h"


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

        Command **temp = realloc(pipeline->command, pipeline->capacity);
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


static void
terminate_pipeline(Pipeline *pipeline)
{
    /* Go to each command in pipeline, terminate it if its running */
}


static int
wait_for_pipeline(Pipeline *pipeline)
{
    for (;;) {
        int status;
        pid_t child_pid = wait(&status);

        if (child_pid == -1) {
            if (errno == ECHILD) {
                /* no more children to wait for */
                break;
            }
            else {
                /* wait fails */
                return -1;
            }
        }

        /* Find the command with the specified pid and update its status */
        for (int i = 0; i < pipeline->command_count; i++) {
            if (pipeline->command[i]->pid == child_pid) {
                pipeline->command[i]->pid  = 0;
                pipeline->command[i]->pgid = 0;
                pipeline->command[i]->return_status = status;
            }
        }
    }

    return 0;
}


int
launch_pipeline(Pipeline *pipeline)
{
    int pipefd[2];
    int infile = STDIN_FILENO;
    int outfile;

    int shell_term = open("/dev/tty", O_RDONLY);

    for (int i = 0; i < pipeline->command_count; i++) {
        if (i + 1 == pipeline->command_count) {
            /* Stdout as outfile for last command in pipeline */
            outfile = STDOUT_FILENO;
        }
        else {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                terminate_pipeline(pipeline);
                return -1;
            }
            /* Connect outfile to the write end of pipe */
            outfile = pipefd[1];
        }

        pid_t pid = fork();

        switch (pid) {
            case 0: /* child process */
                /* Change the process group */
                pid = getpid();
                pid_t pgid = pipeline->gid;
                if (pgid == 0) {
                    /* If the first process in group */
                    pgid = pid;
                }
                setpgid(pid, pgid);

                launch_command(pipeline->command[i], infile, outfile);

            case -1: /* fork fails */
                terminate_pipeline(pipeline);
                return -1;

            default: /* parent process */
                /* Change process group of child */
                if (pipeline->gid == 0) {
                    pipeline->gid = pid;
                }
                setpgid(pid, pipeline->gid);

                pipeline->command[i]->pid  = pid;
                pipeline->command[i]->pgid = pipeline->gid;
                // pipeline->command[i]->running = true;
        }

        /* Clean up after setting up pipes */
        if (infile != STDIN_FILENO) {
            close(infile);
        }
        if (outfile != STDOUT_FILENO) {
            close(outfile);
        }
        infile = pipefd[0];
    }

    tcsetpgrp(shell_term, pipeline->gid);

        /* If pipeline is not part of subshell, it means it should 
          exist as independent job. If its part of subshell, it is
          handled by the subshell. */

        // TODO: Logic for adding pipeline to job list

    wait_for_pipeline(pipeline);
    put_shell_in_foreground();

    /* return the status of last command in pipeline */
    return pipeline->command[pipeline->command_count - 1]->return_status;
}