#include <stdlib.h>
#include <stdio.h>

#include "command.h"


Command *
get_cmd_obj(void)
{
    Command *command = malloc(sizeof(*command));
    if (command == NULL) {
        perror("Command object initialization");
        return NULL;
    }

    command->argv     = NULL;
    command->argc     = 0;
    command->capacity = 0;
    return command;
}


void
destroy_cmd_obj(Command *command)
{
    for (int i = 0; i < command->argc; i++) {
        /* Free each array of characters */
        free(command->argv[i]);
    }

    free(command);
}


int
add_arg_to_cmd_obj(Command *command, const char *arg)
{
    #define INCR_SIZE 2

    /* Expand array if capacity is not sufficient */
    if (command->capacity <= command->argc) {
        command->capacity = command->capacity == 0
                          ? INCR_SIZE : command->capacity + INCR_SIZE;

        char **argv = realloc(command->argv, command->capacity);
        if (argv == NULL) {
            perror("Adding arg to command object");
            command->capacity -= INCR_SIZE;  /* reset capacity */
            return -1;
        }

        command->argv = argv;
    }

    command->argv[command->argc] = arg;
    command->argc += 1;
    return 0;

    #undef INCR_SIZE
}
