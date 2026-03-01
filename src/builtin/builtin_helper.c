#include "builtin_helper.h"

#include <assert.h>
#include <stdlib.h>
#include <errno.h>


int
parse_args(char **argv, int argc)
{
    assert(argc > 0 && argv != NULL);

    /* Each process has atleast 2 args = process
       name and NULL */

    /* Some extra arguments are provided */
    if (argc > 3) {
        return -1;
    }

    /* No argument was provided, meaning the first job */
    if (argc == 2) {
        return 1;
    }

    // BUG: TODO: Long to int conversion may lead to error
    char *str = argv[1];
    char *nptr;
    errno = 0;
    int num = (int) strtol(str, &nptr, 10);
    if (errno == ERANGE || *nptr != '\0') {
       return -1;
    }
    return num;
}
