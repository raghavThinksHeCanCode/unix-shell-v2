#include <stddef.h>
#include <stdio.h>

#include "parser.h"
#include "ast.h"
#include "exec_unit.h"
#include "job.h"
#include "process.h"
#include "token.h"


static Process_obj *parse_process(Token *tokens, int *current);
static Ast_node *parse_job(Token *tokens, int *current);
static Ast_node *parse_condition(Token *tokens, int *current);
static Execution_unit *parse_sequence(Token *tokens, int *current);


/* Individual command and its args */
static Process_obj *
parse_process(Token *tokens, int *current)
{
    if (tokens[*current].type != NAME) {
        fprintf(stderr, "Syntax error\n");
        return NULL;
    }

    Process_obj *proc_obj = get_process_obj();
    if (proc_obj == NULL) {
        return NULL;
    }

    while (tokens[*current].type == NAME) {
        /* Add the current lexeme of the token as argument of process */
        if (add_arg_to_process(proc_obj, tokens[*current].lexeme) == -1) {
            destroy_process_obj(proc_obj);
            return NULL;
        }
        *current += 1;
    }

    return proc_obj;
}


/* Pipeline */
static Ast_node *
parse_job(Token *tokens, int *current)
{
    Job_obj *job_obj = get_job_obj();
    if (job_obj == NULL) {
        return NULL;
    }

    Process_obj *proc_obj = parse_process(tokens, current);
    if (proc_obj == NULL) {
        destroy_job_obj(job_obj);
        return NULL;
    }

    /* Adding process to job fails */
    if (add_process_to_job(job_obj, proc_obj) == -1) {
        destroy_process_obj(proc_obj);
        destroy_job_obj(job_obj);
        return NULL;
    }

    while (tokens[*current].type == PIPE) {
        *current += 1;

        proc_obj = parse_process(tokens, current);
        if (proc_obj == NULL) {
            destroy_job_obj(job_obj);
            return NULL;
        }

        if (add_process_to_job(job_obj, proc_obj) == -1) {
            destroy_process_obj(proc_obj);
            destroy_job_obj(job_obj);
            return NULL;
        }
    }

    /* Create a node of type `JOB` with pointer to `job_obj` */
    Ast_node *node = create_ast_node(JOB, job_obj);
    if (node == NULL) {
        destroy_job_obj(job_obj);
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


/* Parse `&` and `;` */
static Execution_unit *
parse_sequence(Token *tokens, int *current)
{
    /* `head` is the head of the `Execution_unit` linked list */
    Execution_unit *head = get_exec_unit_node();
    if (head == NULL) {
        return NULL;
    }

    Execution_unit *ptr = head;
    Ast_node *ast_root  = parse_condition(tokens, current);
    if (ast_root == NULL) {
        destroy_exec_unit_list(head);
        return NULL;
    }

    ptr->ast_root = ast_root;

    while (tokens[*current].type == AMPRSND
        || tokens[*current].type == SEMICLN) {

        /* Last unit should be in background */
        if (tokens[*current].type == AMPRSND) {
            ptr->context = BACKGROUND;
        }

        *current += 1;

        /* For scenerio like `command &` or
            `command ;`, below will not execute */
        if (tokens[*current].type != NIL) {
            Execution_unit *temp = get_exec_unit_node();
            if (temp == NULL) {
                destroy_exec_unit_list(head);
                return NULL;
            }

            /* Add temp to next of ptr and set ptr to temp */
            ptr->next = temp;
            ptr       = temp;
            ast_root  = parse_condition(tokens, current);
            if (ast_root == NULL) {
                destroy_exec_unit_list(head);
                return NULL;
            }
            ptr->ast_root = ast_root;
        }
    }

    return head;
}


Execution_unit *
parse_tokens(Token *tokens)
{
    int current = 0;
    Execution_unit *head = parse_sequence(tokens, &current);
    if (head == NULL) {
        return NULL;
    }

    /* If not all tokens are parsed */
    if (tokens[current].type != NIL) {
        destroy_exec_unit_list(head);
        return NULL;
    }

    return head;
}