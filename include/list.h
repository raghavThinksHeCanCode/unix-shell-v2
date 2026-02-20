/*
    List is simply a linked list where each node
    has its own AST. This design allows dividing
    nodes to subshells that can execute in background.

    A node is added in the list, if the original Process
    has a `;` or a `&`, latter of which represents background
    execution.
*/

#ifndef LIST_H_
#define LIST_H_


#include "ast.h"
#include <stdbool.h>


typedef struct List_node
{
    bool              is_foreground;
    Ast_node         *ast_root;
    struct List_node *next;
} List_node;


List_node *get_list_node(void);
void destroy_list(List_node *head);


#endif // LIST_H_