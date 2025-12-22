#ifndef COMMAND_H_
#define COMMAND_H_


#include <stddef.h>


/*
    `Cmd_table` stores commands in a linear fashion.
    Each command is a `char**`, represented like {"ls", "-al", NULL}.
    Query the command table by index to get the command.
*/

typedef char** Command;

typedef struct Cmd_table
{
    Command *command;  /* to store the available commands */
    int count;      /* total commands in the table */
    int capacity;   /* capacity of table */
} Cmd_table;


Cmd_table *init_cmd_table(void);
int write_command(Cmd_table *cmd_table, const Command command);
void destroy_cmd_table(Cmd_table *cmd_table);
void destroy_command(Command command);


#endif // COMMAND_H_
