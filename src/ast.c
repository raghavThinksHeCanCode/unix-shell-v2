#include <stdio.h>
#include <stdlib.h>

#include "ast.h"


Ast_node *
create_ast_node(Node_type type, int pipeline_index)
{
    Ast_node *node = malloc(sizeof(*node));
    if (node == NULL) {
        perror("create_ast_node");
        return NULL;
    }

    node->type           = type;
    node->pipeline_index = pipeline_index;

    /* Default fields */
    node->left       = NULL;
    node->right      = NULL;
    node->return_val = 0;

    return node;
}


void
destroy_ast(Ast_node *ast_root)
{
    /* Free nodes recursively */
    if (ast_root == NULL) {
        return;
    }

    destroy_ast(ast_root->left);
    destroy_ast(ast_root->right);
    free(ast_root);
}