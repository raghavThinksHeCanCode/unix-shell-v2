#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "command.h"


Command *
get_command_obj(void)
{
    Command *command = malloc(sizeof(*command));
    if (command == NULL) {
        perror("get_command_obj");
        return NULL;
    }

    command->argv     = NULL;
    command->argc     = 0;
    command->capacity = 0;
    return command;
}


void
destroy_command_obj(Command *command)
{
    /* Free every `char *` in argv array */
    for (int i = 0; i < command->argc; i++) {
        free(command->argv[i]);
    }

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
        
        char **temp = realloc(command->argv, command->capacity);
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