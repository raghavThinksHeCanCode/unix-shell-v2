#include "builtin_helper.h"
#include "job.h"
#include "shell.h"

#include <stdlib.h>


int
builtin_fg(char **argv, int argc)
{
    Job *job_head = get_job_head();
    if (job_head != NULL) {
        put_job_in_foreground(job_head);
    }
    return 0;
}
