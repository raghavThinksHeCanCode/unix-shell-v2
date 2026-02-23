#include "builtin.h"

#include <assert.h>
#include <string.h>


Builtin
match_builtin(char **argv)
{
    assert(argv != NULL && argv[0] != NULL);

    if (!strcmp(argv[0], "fg")) return BUILTIN_FG;
    if (!strcmp(argv[0], "bg")) return BUILTIN_BG;
    if (!strcmp(argv[0], "jobs")) return BUILTIN_JOBS;

    return BUILTIN_NONE;
}


int
exec_builtin(Builtin builtin, char **argv, int argc)
{
    assert(builtin != BUILTIN_NONE);
    assert(argv != NULL && argv[0] != NULL);

    int return_val;

    switch (builtin) {
        case BUILTIN_FG:
            return_val = builtin_bg(argv, argc);
            break;

        case BUILTIN_BG:
            return_val = builtin_fg(argv, argc);
            break;

        case BUILTIN_JOBS:
            return_val = builtin_jobs(argv, argc);
            break;

        default:
            return_val = 1;
            break;
    }

    return return_val;
}