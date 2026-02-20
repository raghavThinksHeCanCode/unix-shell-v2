#include "shell.h"
#include "executor.h"
#include "input.h"
#include "lexer.h"
#include "list.h"
#include "parser.h"
#include "token.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>


static void set_signals_to_ignore(void);
static int put_shell_in_new_group(void);
static int init_shell(void);
static void start_shell_loop(void);


static pid_t shell_pgid;
static int   shell_terminal;


int
get_shell_terminal(void)
{
    return shell_terminal;
}


static void
set_signals_to_ignore(void)
{
    struct sigaction action;

    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    // sigaction(SIGINT, &action, NULL);  for now turning it off
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
    sigaction(SIGTTIN, &action, NULL);
    sigaction(SIGTTOU, &action, NULL);
}


static int
put_shell_in_new_group(void)
{
    shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) == -1) {
        perror("Couldn't put the shell in new process group");
        return -1;
    }

    return 0;
}


void
put_shell_in_foreground(void)
{
    tcsetpgrp(shell_terminal, shell_pgid);
}


#define IS_SHELL_IN_FOREGROUND() \
        (tcgetpgrp(shell_terminal) == getpgrp())  /* compare foreground group with shell's group */

static int
init_shell(void)
{
    shell_terminal = open("/dev/tty", O_RDONLY);

    while (!IS_SHELL_IN_FOREGROUND()) {
        /* Stop the process group the shell belongs to if started in background */
        kill(0, SIGTTIN);        
    }

    set_signals_to_ignore();
    if (put_shell_in_new_group() == -1) {
        return -1;
    }

    /* Grab control of the terminal */
    put_shell_in_foreground();
    return 0;
}

#undef IS_SHELL_IN_FOREGROUND


static void
start_shell_loop(void)
{
    while (1) {
        printf("> ");
        char *line = read_from_stdin();
        if (line[0] == '\0' || line == NULL) {
            /* If user clicks enter without typing anything or on error */
            free(line);
            continue;
        }

        Token *tokens = tokenize(line);
        if (tokens == NULL) {
            free(line);
            continue;
        }

        List_node *list_head = parse_tokens(tokens);
        if (list_head == NULL) {
            free_tokens(tokens);
            free(line);
            continue;
        }

        execute(list_head);

        destroy_list(list_head);
        free_tokens(tokens);
        free(line);
    }
}


int
start_shell(void)
{
    if (init_shell() == -1) {
        return EXIT_FAILURE;
    }

    start_shell_loop();
    return EXIT_SUCCESS;
}