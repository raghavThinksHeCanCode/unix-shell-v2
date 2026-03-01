#include "builtin_helper.h"
#include "job.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


int
builtin_fg(char **argv, int argc)
{
    Job *job_head = get_job_head();
    if (job_head == NULL) {
        fprintf(stderr, "shell: fg: No job running\n");
        return 1;
    }

    int job_number;
    if ((job_number = parse_args(argv, argc)) == -1) {
        fprintf(stderr, "shell: fg: Invalid args\n");
        return 1;
    }

    Job *job_node;
    if ((job_node = find_job_with_number(job_number)) == NULL) {
        fprintf(stderr, "shell: fg: No job with job number %d exists", job_number);
        return 1;
    }

    bool cont = true;
    put_job_in_foreground(job_node, cont);
    return 0;
}
