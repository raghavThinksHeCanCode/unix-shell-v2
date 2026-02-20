/*
    Representation of a pipeline.

    `Process` member represents an array of
    `Process *`. Each member of this array
    represents a process which is part of the 
    pipeline.
*/


#ifndef PIPELINE_H_
#define PIPELINE_H_


#include "process.h"

#include <sys/types.h>


/* Struct representing a pipeline */
typedef struct Pipeline
{
   //TODO: update this struct to include fields like running and stopped
    Process **process;  /* each process of this array is part of pipeline */
    pid_t     gid;      /* process group id of the pipeline */
    int       process_count;
    bool      is_running;
    int       capacity;
} Pipeline;


Pipeline *get_pipeline_obj(void);
void destroy_pipeline_obj(Pipeline *pipeline);
int add_process_to_pipeline(Pipeline *pipeline, Process *process);
int launch_pipeline(Pipeline *pipeline);


#endif // PIPELINE_H_
