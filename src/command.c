#include "command.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


Command *
get_command_obj(void)
{
    Command *command = malloc(sizeof(*command));
    if (command == NULL) {
        perror("get_command_obj");
        return NULL;
    }

    command->argv          = NULL;
    command->argc          = 0;
    command->capacity      = 0;
    // command->is_running    = false;
    // command->pid           = -1;
    command->return_status = 0;
    return command;
}


void
destroy_command_obj(Command *command)
{
    /* Free the argv array */
    free(command->argv);

    /* Free the structure */
    free(command);
}


int
add_arg_to_command(Command *command, const char *arg)
{
    #define INCR_SIZE 2

    if (command->capacity <= command->argc) {
        /* Increase capacity for insufficient size */
        command->capacity += INCR_SIZE;
        
        char **temp = realloc(command->argv, sizeof(*temp) * (command->capacity));
        if (temp == NULL) {
            command->capacity -= 1; /* reset capacity */
            perror("add_arg_to_command");
            return -1;
            /* command->argv still points to prev memory */
        }
    
        command->argv = temp;
    }

    command->argv[command->argc] = arg;
    command->argc += 1;

    /* Return index where arg is stored */
    return command->argc - 1;

    #undef INCR_SIZE
}


void
update_command_status(Command *command, bool is_running, pid_t pid)
{
    // command->is_running = is_running;
    // command->pid        = pid;
}


static void
reset_signal_disposition(void)
{
    struct sigaction action;

    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
    sigaction(SIGTTIN, &action, NULL);
    sigaction(SIGTTOU, &action, NULL);
}


#define UPDATE_FDS(infile, outfile)        \
        if (infile != STDIN_FILENO) {      \
            dup2(infile, STDIN_FILENO);    \
            close(infile);                 \
        }                                  \
        if (outfile != STDOUT_FILENO) {    \
            dup2(outfile, STDOUT_FILENO);  \
            close(outfile);                \
        }

void
launch_command(Command *command, int infile, int outfile)
{
    UPDATE_FDS(infile, outfile);
    reset_signal_disposition();

    char **argv = command->argv;
    execvp(argv[0], argv);

    /* execvp fails */
    char buf[32];
    snprintf(buf, sizeof(buf), "%s", argv[0]);
    perror(buf);
    _exit(EXIT_FAILURE);
}

#undef UPDATE_FDS