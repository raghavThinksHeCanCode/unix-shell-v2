/*
    Representation of a pipeline.

    `command` member represents an array of
    `Command *`. Each member of this array
    represents a process which is part of the 
    pipeline.
*/


#ifndef PIPELINE_H_
#define PIPELINE_H_


#include "command.h"


/* Struct representing a pipeline */
typedef struct Pipeline
{
    Command **command;  /* each process of this array is part of pipeline */
    int       command_count;
    int       capacity;
} Pipeline;


Pipeline *get_pipeline_obj(void);
void destroy_pipeline_obj(Pipeline *pipeline);
int add_command_to_pipeline(Pipeline *pipeline, Command *command);


#endif // PIPELINE_H_
