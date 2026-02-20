/*
   A Command is a command that the shell needs to execute.
   It is defined as the struct Command.

   `argv` member of the struct is an array of strings, which 
   represents the the command to execute and its arguments.
   The last member of this array must be `NULL`. This is to 
   make it using with `exec` easy.

   `argc` is simply the number of elements in `argv` including
   `NULL`. `capacity` is the space in `argv` array.
*/


#ifndef COMMAND_H_
#define COMMAND_H_


#include <stdbool.h>
#include <sys/types.h>


/* Struct to hold info about a command/process */
typedef struct Command
{
   //TODO: update this struct to include fields like running and stopped
   char **argv;     /* something like `{"ls", "-al", NULL}` */
   int    argc;
   int    return_status;
   pid_t  pid;
   pid_t  pgid;
   int    capacity; /* not to be used */
} Command;


/* Returns a pointer to default-initialized
   `Command`. Returns `NULL` on error. */
Command *get_command_obj(void);

void destroy_command_obj(Command *command);

/* Add another argument to argv array. 
   On success, returns index where the 
   arg was added. On failure, returns -1. */
int add_arg_to_command(Command *command, const char *arg);

void update_command_status(Command *command, bool is_running, pid_t pid);

void launch_command(Command *command, int infile, int outfile);


#endif // COMMAND_H_