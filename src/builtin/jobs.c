#include "builtin_helper.h"
#include "job.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>


int
builtin_jobs(char **argv, int argc)
{
    assert(!strcmp(argv[0], "jobs"));

    if (argv[1] != NULL) {
        fprintf(stderr, "jobs: Not expected any arguments\n");
        return 1;
    }
    
    Job *job_head = get_job_head();

    if (job_head == NULL) {
        printf("shell: There are currently no jobs running\n");
        return 1;
    }

    int job_number = 1;
    while (job_head != NULL) {
        printf("[%d] %s     *Job No. %d*\n", job_number,
                job_head->is_stopped ? "Stopped" : "Running", job_number);

        job_number += 1;
        job_head = job_head->next;
    }

    return 0;
}