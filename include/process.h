/*
   A Process is a process that the shell needs to execute.
   It is defined as the struct Process.

   `argv` member of the struct is an array of strings, which 
   represents the process to execute and its arguments.
   The last member of this array must be `NULL`. This is to 
   make it using with `exec` easy.

   `argc` is simply the number of elements in `argv` including
   `NULL`. `capacity` is the space in `argv` array.
*/


#ifndef Process_H_
#define Process_H_


#include <stdbool.h>
#include <sys/types.h>


/* Struct to hold info about a Process/process */
typedef struct Process
{
   char **argv;     /* something like `{"ls", "-al", NULL}` */
   int    argc;
   int    return_val;

   pid_t  pid;      /* process ID of the process */
   pid_t  pgid;     /* group ID where the process belong */

   int    capacity; /* not for application use */
} Process;


/* Returns a pointer to default-initialized
   `Process`. Returns `NULL` on error. */
Process *get_process_obj(void);

void destroy_process_obj(Process *process);

/* Add another argument to argv array. 
   On success, returns index where the 
   arg was added. On failure, returns -1. */
int add_arg_to_process(Process *process, const char *arg);

void launch_process(Process *process, int infile, int outfile);


#endif // Process_H_