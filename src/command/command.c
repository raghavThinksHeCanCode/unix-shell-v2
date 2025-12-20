#include <stdio.h>
#include <stdlib.h>

#include "command.h"


Cmd_table *
init_cmd_table(void)
{
    Cmd_table *cmd_table = malloc(sizeof(*cmd_table));
    if (cmd_table == NULL) {
        perror("Init Command Table");
        return NULL;
    }

    cmd_table->capacity = 0;
    cmd_table->count    = 0;
    cmd_table->command  = NULL;
    return cmd_table;
}
