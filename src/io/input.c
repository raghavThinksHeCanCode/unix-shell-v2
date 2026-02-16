#include "input.h"

#include <stdio.h>
#include <sys/types.h>


char *
read_from_stdin(void)
{
    char *line = NULL;
    size_t n   = 0;
    ssize_t line_len = getline(&line, &n, stdin);

    if (line_len == -1) {
        perror("Command Reading failed");
        return NULL;
    }

    /* Replace delim with null-byte */
    if (line[line_len - 1] == '\n') {
        line[line_len - 1] = '\0';
    }

    return line;  /* buffer pointed by line must be freed by the caller */
}
