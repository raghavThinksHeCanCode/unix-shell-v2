/*
    A job is simply a pipeline of processes.
    It is represented using `struct Job_obj`.

    `process` member represents an array of
    `Process_obj *`. Each member of this array
    represents a process which is part of the 
    pipeline or job.
*/


#ifndef JOB_H_
#define JOB_H_


#include "process.h"


/* Struct representing a job, which
   is a pipeline of processes */
typedef struct Job_obj
{
    Process_obj **process;  /* each process of this array is part of pipeline */
    int           process_count;
    int           capacity;
} Job_obj;


Job_obj *get_job_obj(void);
void destroy_job_obj(Job_obj *job_obj);
int add_process_to_job(Job_obj *job_obj, Process_obj *process_obj);


#endif // JOB_H_
