/*
    In order to understand what an instance is
    we first need to understand why we needed it.
    If you read the file `ast.h`, you will notice
    that there is no node of type `&`, which represents
    a background operation. This is because the `&` is
    handled by introducing another layer of abstraction, i.e,
    the `Instance`.

    Take for example this line:
    `ls && whomai & ping site.com & cat file`

    This line will have 3 instances:
    `ls && whoami` which will be executed in background
    `ping site.com` which will also be executed in background
    `cat file` which will be executed in foreground.

    These 3 instances will execute simultaneously, two of which
    will be executed in the background, while the last will be
    in the foreground. Each of these instance will have its own
    ast which the shell would need to traverse and execute.

    But the shell can only traverse a single ast at a time. So
    to make background operation possible, the shell will spawn
    child shells (two in this case), that will execute the background
    instances.
*/

#ifndef INSTANCE_H_
#define INSTANCE_H_


#include "ast.h"


/* An instance will either be executed
   in background or foreground */
typedef enum Execution_context
{
    FOREGROUND, BACKGROUND,
} Execution_context;


/* An instance is a linked list, that points
   to the next instance to handle or `NULL`*/
typedef struct Instance
{
    Execution_context context;
    Ast_node         *ast_root;
    struct Instance  *next;
} Instance;


#endif // INSTANCE_H_