#include <stdio.h>
#include <stdlib.h>

#include "pipeline.h"
#include "command.h"


Pipeline *
get_pipeline_obj(void)
{
    Pipeline *pipeline = malloc(sizeof(*pipeline));
    if (pipeline == NULL) {
        perror("get_pipeline_obj");
        return NULL;
    }

    pipeline->command       = NULL;
    pipeline->command_count = 0;
    pipeline->capacity      = 0;
    return pipeline;
}


void
destroy_pipeline_obj(Pipeline *pipeline)
{
    for (int i = 0; i < pipeline->command_count; i++) {
        /* Destroy each command in the pipeline */
        destroy_command_obj(pipeline->command[i]);
    }

    free(pipeline->command);
    free(pipeline);
}


int
add_command_to_pipeline(Pipeline *pipeline, Command *command)
{
    #define INCR_SIZE 1

    if (pipeline->capacity <= pipeline->command_count) {
        pipeline->capacity += INCR_SIZE;

        Command **temp = realloc(pipeline->command, pipeline->capacity);
        if (temp == NULL) {
            pipeline->capacity -= 1; /* reset capacity */
            perror("add_command_to_pipeline");
            return -1;
        }

        pipeline->command = temp;
    }

    /* Store the command obj in array */
    pipeline->command[pipeline->command_count] = command;
    pipeline->command_count += 1;

    /* Return index where the command was stored */
    return pipeline->command_count - 1;

    #undef INCR_SIZE
}