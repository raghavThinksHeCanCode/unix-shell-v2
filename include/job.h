#ifndef JOB_H_
#define JOB_H_


#include <stddef.h>


/*
    Each job is an array that stores that
    stores the index of all commands in a
    job. Use command table to get the command.
*/
typedef struct Job
{
    size_t *cmd_address;
    size_t  count;
    size_t  capacity;
} Job;

typedef struct Job_table
{
    Job *job;
    size_t count;
    size_t capacity;
} Job_table;


#endif // JOB_H_