#include "job.h"
#include "pipeline.h"

#include <stdlib.h>
#include <stdio.h>


static int get_job_number(Job *job_node);
static void add_node_to_job_list(Job *job_node);


/* Jobs will be stored in a linked list, of
   which job_head is the head */
static Job *job_head = NULL;


static int
get_job_number(Job *job_node)
{
    int job_number = 1;
    Job *temp      = job_head;

    while (temp != job_node) {
        temp        = temp->next;
        job_number += 1;
    }
    return job_number;
}


/* Add the job_node to end of the job list */
static void
add_node_to_job_list(Job *job_node)
{
    if (job_head == NULL) {
        job_head = job_node;
        return;
    }

    Job *ptr = job_head;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    ptr->next = job_node;
}


Job *
add_pipeline_to_job(Pipeline *pipeline, bool is_stopped, bool in_foreground)
{
    Job *job_node = malloc(sizeof(*job_node));
    if (job_node == NULL) {
        perror("Creating job out of pipeline failed");
        return NULL;
    }

    job_node->next          = NULL;
    job_node->gid           = pipeline->gid;
    job_node->process       = pipeline->process;
    job_node->process_count = pipeline->process_count;
    job_node->is_stopped    = is_stopped;
    job_node->in_foreground = in_foreground;
    job_node->is_completed  = false;

    add_node_to_job_list(job_node);
    return job_node;
}


void
notify_job_status(Job *job)
{
    printf("\n[%d] %s     *job processes go here*\n", get_job_number(job), 
                job->is_stopped ? "Stopped" : "Running");
}