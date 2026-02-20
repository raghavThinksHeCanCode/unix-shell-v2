#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "parser.h"
#include "ast.h"
#include "list.h"
#include "pipeline.h"
#include "process.h"
#include "token.h"


static Process   *parse_process(Token *tokens, int *current);
static Ast_node  *parse_pipeline(Token *tokens, int *current);
static Ast_node  *parse_condition(Token *tokens, int *current);
static List_node *parse_sequence(Token *tokens, int *current);


/* Individual Process and its args */
static Process *
parse_process(Token *tokens, int *current)
{
    if (tokens[*current].type != NAME) {
        fprintf(stderr, "Syntax error\n");
        return NULL;
    }

    Process *process = get_process_obj();
    if (process == NULL) {
        return NULL;
    }

    while (tokens[*current].type == NAME) {
        /* Add the current lexeme of the token as argument of Process */
        if (add_arg_to_process(process, tokens[*current].lexeme) == -1) {
            destroy_process_obj(process);
            return NULL;
        }
        *current += 1;
    }

    if (add_arg_to_process(process, NULL) == -1) {
        /* Add `NULL` as the last argument */
        destroy_process_obj(process);
        return NULL;
    }

    return process;
}


/* Pipeline */
static Ast_node *
parse_pipeline(Token *tokens, int *current)
{
    Pipeline *pipeline = get_pipeline_obj();
    if (pipeline == NULL) {
        return NULL;
    }

    Process *process = parse_process(tokens, current);
    if (process == NULL) {
        destroy_pipeline_obj(pipeline);
        return NULL;
    }

    /* Adding Process to pipeline fails */
    if (add_process_to_pipeline(pipeline, process) == -1) {
        destroy_process_obj(process);
        destroy_pipeline_obj(pipeline);
        return NULL;
    }

    while (tokens[*current].type == PIPE) {
        *current += 1;

        process = parse_process(tokens, current);
        if (process == NULL) {
            destroy_pipeline_obj(pipeline);
            return NULL;
        }

        if (add_process_to_pipeline(pipeline, process) == -1) {
            destroy_process_obj(process);
            destroy_pipeline_obj(pipeline);
            return NULL;
        }
    }

    /* Create a node of type `PIPELINE` with pointer to `pipeline` */
    Ast_node *node = create_ast_node(PIPELINE, pipeline);
    if (node == NULL) {
        destroy_pipeline_obj(pipeline);
        return NULL;
    }

    return node;
}


#define GET_NODE_TYPE(tokens, current) \
        tokens[*current].type == DOUBLE_AMPRSND \
        ? AND : OR

/* `&&` and `||` */
static Ast_node *
parse_condition(Token *tokens, int *current)
{
    Ast_node *left = parse_pipeline(tokens, current);
    if (left == NULL) {
        return NULL;
    }

    while (tokens[*current].type == DOUBLE_AMPRSND
        || tokens[*current].type == DOUBLE_PIPE) {

        Node_type root_type = GET_NODE_TYPE(tokens, current);
        *current += 1;

        Ast_node *right = parse_pipeline(tokens, current);
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


/* Parse `&` and `;` */
static List_node *
parse_sequence(Token *tokens, int *current)
{
    /* `head` is the head of the `List_node` linked list */
    List_node *head = get_list_node();
    if (head == NULL) {
        return NULL;
    }

    List_node *ptr = head;
    Ast_node *ast_root  = parse_condition(tokens, current);
    if (ast_root == NULL) {
        destroy_list(head);
        return NULL;
    }

    ptr->ast_root = ast_root;

    while (tokens[*current].type == AMPRSND
        || tokens[*current].type == SEMICLN) {

        /* Last node should be in background */
        if (tokens[*current].type == AMPRSND) {
            ptr->is_foreground = false;
        }

        *current += 1;

        /* For scenerio like `command &` or
            `command ;`, below will not execute */
        if (tokens[*current].type != NIL) {
            List_node *temp = get_list_node();
            if (temp == NULL) {
                destroy_list(head);
                return NULL;
            }

            /* Add temp to next of ptr and set ptr to temp */
            ptr->next = temp;
            ptr       = temp;
            ast_root  = parse_condition(tokens, current);
            if (ast_root == NULL) {
                destroy_list(head);
                return NULL;
            }
            ptr->ast_root = ast_root;
        }
    }

    return head;
}


List_node *
parse_tokens(Token *tokens)
{
    int current = 0;
    List_node *head = parse_sequence(tokens, &current);
    if (head == NULL) {
        return NULL;
    }

    /* If not all tokens are parsed */
    if (tokens[current].type != NIL) {
        fprintf(stderr, "Syntax Error\n");
        destroy_list(head);
        return NULL;
    }

    return head;
}