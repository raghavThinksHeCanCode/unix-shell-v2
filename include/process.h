/*
   A process is a command that the shell needs to execute.
   It is defined as the struct Process_obj. 

   `argv` member of the struct is an array of strings, which 
   represents the the command to execute and its arguments.
   The last member of this array must be `NULL`. This is to 
   make it using with `exec` easy.

   `argc` is simply the number of elements in `argv` including
   `NULL`. `capacity` is the space in `argv` array.
*/


#ifndef PROCESS_H_
#define PROCESS_H_


/* Struct to holds info about a command/process */
typedef struct Process_obj
{
    char **argv;     /* something like `{"ls", "-al", NULL}` */
    int    argc;
    int    capacity;

} Process_obj;


/* Returns a pointer to default-initialized
   `Process_obj`. Returns `NULL` on error. */
Process_obj *get_process_obj(void);

void destroy_process_obj(Process_obj *process_obj);

/* Add another argument to argv array. 
   On success, returns index where the 
   arg was added. On failure, returns -1. */
int add_arg_to_process(Process_obj *process_obj, const char *arg);


#endif // PROCESS_H_