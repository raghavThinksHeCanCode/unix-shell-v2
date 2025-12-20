#ifndef COMMAND_H_
#define COMMAND_H_


#include <stddef.h>


typedef char** Command;

/*
    `Cmd_table` stores commands in a linear fashion.
    Each command is a `char**`, represented like {"ls", "-al", NULL}.
    Query the command table by index to get the command.
*/
typedef struct Cmd_table
{
    Command *command;  /* to store the available commands */
    size_t count;      /* total commands in the table */
    size_t capacity;   /* capacity of table */
} Cmd_table;


#endif // COMMAND_H_
