#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>


void
set_signals_to_ignore(void)
{
    struct sigaction action;

    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);
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