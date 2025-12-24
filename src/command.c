#include <stdlib.h>
#include <stdio.h>

#include "command.h"


Command *
get_cmd_obj(void)
{
    Command *command_obj = malloc(sizeof(*command_obj));
    if (command_obj == NULL) {
        perror("Command object initialization");
        return NULL;
    }

    command_obj->argv     = NULL;
    command_obj->argc     = 0;
    command_obj->capacity = 0;
    return command_obj;
}


void
destroy_cmd_obj(Command *command_obj)
{
    for (int i = 0; i < command_obj->argc; i++) {
        /* Free each array of characters */
        free(command_obj->argv[i]);
    }

    free(command_obj);
}


int
add_arg_to_cmd(Command *command_obj, const char *arg)
{
    #define INCR_SIZE 2

    /* Expand array if capacity is not sufficient */
    if (command_obj->capacity <= command_obj->argc) {
        command_obj->capacity += INCR_SIZE;

        char **argv = realloc(command_obj->argv, command_obj->capacity);
        if (argv == NULL) {
            perror("Adding arg to command object");
            command_obj->capacity -= INCR_SIZE;  /* reset capacity */
            return -1;
        }

        command_obj->argv = argv;
    }

    command_obj->argv[command_obj->argc] = arg;
    command_obj->argc += 1;
    return (command_obj->argc - 1);

    #undef INCR_SIZE
}
