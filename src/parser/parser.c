#include <stddef.h>
#include <stdlib.h>

#include "parser.h"
#include "ast.h"
#include "instance.h"
#include "token.h"


static Ast_node *parse_job(Token *tokens, int *current);
static Ast_node *parse_condition(Token *tokens, int *current);
static Instance *parse_instance(Token *tokens, int *current);


#define GET_NODE_TYPE(tokens, current) \
        tokens[*current].type == DOUBLE_AMPRSND \
        ? AND : OR

static Ast_node *
parse_condition(Token *tokens, int *current)
{
    Ast_node *left = parse_job(tokens, current);
    if (left == NULL) {
        return NULL;
    }

    while (tokens[*current].type == DOUBLE_AMPRSND
        || tokens[*current].type == DOUBLE_PIPE) {

        Node_type root_type = GET_NODE_TYPE(tokens, current);
        *current += 1;

        Ast_node *right = parse_job(tokens, current);
        if (right == NULL) {
            destroy_ast(left);
            return NULL;
        }

        Ast_node *root = create_ast_node(root_type, NULL);
        if (root == NULL) {
            destroy_ast(left);
            destroy_ast(right);
            return NULL;
        }

        root->left  = left;
        root->right = right;
        left = root;
    }

    return left;
}

#undef GET_NODE_TYPE


static Instance *
parse_instance(Token *tokens, int *current)
{
    /* `instance` is the head of the `Instance` linked list */
    Instance *instance = get_instance_obj();
    if (instance == NULL) {
        return NULL;
    }

    Instance *ptr      = instance;
    Ast_node *ast_root = parse_condition(tokens, current);
    if (ast_root == NULL) {
        destroy_instance(instance);
        return NULL;
    }

    ptr->ast_root = ast_root;

    while (tokens[*current].type == AMPRSND
        || tokens[*current].type == SEMICLN) {

        /* Last instance should be in background */
        if (tokens[*current].type == AMPRSND) {
            ptr->context = BACKGROUND;
        }

        *current += 1;

        /* For scenerio like `command &` or
            `command ;`, below will not execute */
        if (tokens[*current].type != NIL) {
            Instance *temp = get_instance_obj();
            if (temp == NULL) {
                destroy_instance(instance);
                return NULL;
            }

            /* Managing linked list */
            ptr->next = temp;
            ptr       = temp;
            ast_root  = parse_condition(tokens, current);
            if (ast_root == NULL) {
                destroy_instance(instance);
                return NULL;
            }
            ptr->ast_root = ast_root;
        }
    }

    return instance;
}


Instance *
parse_tokens(Token *tokens)
{
    int current = 0;
    Instance *instance = parse_instance(tokens, &current);
    if (instance == NULL) {
        return NULL;
    }

    /* If not all tokens are parsed */
    if (tokens[current].type != NIL) {
        destroy_instance(instance);
        return NULL;
    }

    return instance;
}