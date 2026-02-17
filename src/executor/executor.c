#include "executor.h"
#include "ast.h"
#include "list.h"
#include "exec_helper.h"
#include "stack.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>


static void traverse_ast(Ast_node *ast_root);
static void handle_list_node(List_node *node);


static void
traverse_ast(Ast_node *ast_root)
{
    /*
        An AST will always have the structure like following:

                                (condition)
                                /         \
                               /           \
                            (condition)   [Pipeline]
                            /         \
                           /           \
                    (condition)   [Pipeline]
                    /         \
                   /           \
                [Pipeline]    [Pipeline]
    */

    Stack *stack   = NULL;
    Ast_node *node = ast_root;

    while (node->type != PIPELINE) {
        /* Push nodes into stack and go to left child until 
           node of type `PIPELINE` is not found */

        if (push_node_into_stack(node, &stack) == -1) {
            destroy_stack(&stack);
            return;
        }
        node = node->left;
    }

    assert(node->type == PIPELINE);
    node->return_status = launch_pipeline(node->pipeline);

    while (stack != NULL) {
        /* Start emptying the stack and execute the right
           child depending on the type of node */

        node = pop_node_from_stack(&stack);
        assert(node->type == AND || node->type == OR);

        /* Current node type will always be `AND` or `OR` and its
           right child will always be of type `PIPELINE` */
        if (can_execute_right_pipeline(node)) {
            node->right->return_status = launch_pipeline(node->right->pipeline);
            update_node_status(node);
        }
    }
}


static void
handle_list_node(List_node *node)
{
    // TODO: Logic for subshell creation goes here

    /* For now, all list nodes are considered as foreground */
    traverse_ast(node->ast_root);
}


void
execute(List_node *head)
{
    for (List_node *node = head; node != NULL; node = node->next) {
        handle_list_node(node);
    }
}