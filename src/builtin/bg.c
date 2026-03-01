#include "builtin_helper.h"
#include "job.h"

#include <stdbool.h>
#include <stdlib.h>


int
builtin_bg(char **argv, int argc)
{
    // TODO: Add args to bg
    /* By default, bg will continue the job in background */
    bool cont = true;
    Job *job_head = get_job_head();
    if (job_head != NULL) {
        put_job_in_background(job_head, cont);
    }
    return 0;
}
