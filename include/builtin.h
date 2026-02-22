#ifndef BUILTIN_H_
#define BUILTIN_H_


typedef enum Builtin
{
    BUILTIN_FG, BUILTIN_BG, BUILTIN_JOBS,

    BUILTIN_NONE,
} Builtin;

Builtin match_builtin(char **argv);
int exec_builtin(Builtin builtin, char **argv, int argc);

int builtin_fg(char **argv, int argc);
int builtin_bg(char **argv, int argc);
int builtin_jobs(char **argv, int argc);


#endif