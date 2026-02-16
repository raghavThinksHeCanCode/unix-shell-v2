#include "shell.h"
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
static int init_shell(void);
static void start_shell_loop(void);


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


#define IS_SHELL_IN_FOREGROUND(shell_terminal) \
        (tcgetpgrp(shell_terminal) == getpgrp())  /* compare foreground group with shell's group */

static int
init_shell(void)
{
    int shell_terminal = open("/dev/tty", O_RDONLY);

    while (!IS_SHELL_IN_FOREGROUND(shell_terminal)) {
        /* Stop the process group the shell belongs to if started in background */
        kill(0, SIGTTIN);        
    }

    set_signals_to_ignore();

    /* Put the shell in its own process group */
    pid_t shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) == -1) {
        perror("Couldn't put the shell in its own process group");
        return EXIT_FAILURE;
    }

    /* Grab control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    return 0;
}


static void
start_shell_loop(void)
{
    // while (1) {
        printf("> ");
        char *line = read_from_stdin();
        if (line[0] == '\0' || line == NULL) {
            /* If use clicks enter without typing anything or on error */
            free(line);
            // continue;
        }

        Token *tokens = tokenize(line);
        if (tokens == NULL) {
            free(line);
            // continue;
        }

        List_node *list_head = parse_tokens(tokens);
        if (list_head == NULL) {
            free_tokens(tokens);
            free(line);
            // continue;
        }

        //
        // TODO: Execution logic goes here
        //

        destroy_list(list_head);
        free_tokens(tokens);
        free(line);
    // }
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