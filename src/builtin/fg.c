#include "builtin_helper.h"
#include "job.h"

#include <stdlib.h>


int
builtin_fg(char **argv, int argc)
{
    // TODO: Add args to choose job
    /* fg will always put job in foreground
       and continue it */
    bool cont = true;

    Job *job_head = get_job_head();
    if (job_head != NULL) {
        put_job_in_foreground(job_head, cont);
    }
    return 0;
}
