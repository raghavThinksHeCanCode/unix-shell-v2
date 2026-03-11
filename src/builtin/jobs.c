#include "builtin_helper.h"
#include "job.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>


int
builtin_jobs(char **argv, int argc)
{
    assert(!strcmp(argv[0], "jobs"));

    if (!is_job_control_enabled()) {
        fprintf(stderr, "shell: jobs: No job control\n");
        return 1;
    }

    if (argv[1] != NULL) {
        fprintf(stderr, "jobs: Not expected any arguments\n");
        return 1;
    }
    
    Job *job_head = get_job_head();

    if (job_head == NULL) {
        printf("shell: There are currently no jobs running\n");
        return 1;
    }

    for (int job_number = 1; job_head != NULL; job_number++) {
        printf("[%d] %s     %s\n", job_number,
                job_head->is_stopped ? "Stopped" : "Running", job_head->string);

        job_head = job_head->next;

    }

    return 0;
}
