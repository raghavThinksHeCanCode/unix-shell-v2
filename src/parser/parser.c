#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "ast.h"
#include "token.h"
#include "pipeline.h"
#include "command.h"
#include "parser_helper.h"


static Command *
parse_primary(Token *tokens, int *current)
{
    if (tokens[*current].type != NAME) {
        fprintf(stderr, "Syntax Error\n");
        return NULL;
    }

    /* `cmd_obj` stores commands and its arguments */
    Command *cmd_obj = get_cmd_obj();
    if (cmd_obj == NULL) {
        return NULL;
    }

    while (tokens[*current].type == NAME) {
        if (add_arg_to_cmd(cmd_obj, tokens[*current].lexeme) == -1) {
            destroy_cmd_obj(cmd_obj);
            return NULL;
        }
        *current += 1;
    }

    /* Add `NULL` as the last argument of command */
    if (add_arg_to_cmd(cmd_obj, NULL) == -1) {
        destroy_cmd_obj(cmd_obj);
        return NULL;
    }

    return cmd_obj;
}


static Ast_node *
parse_pipeline(Token *tokens, int *current, Pipeline_table *pipeline_table)
{
    /* `pipeline_obj` stores all commands in a single pipeline */
    Pipeline *pipeline_obj = get_pipeline_obj();
    if (pipeline_obj == NULL) {
        return NULL;
    }

    Command *cmd_obj = parse_primary(tokens, current);
    if (cmd_obj == NULL) {
        destroy_pipeline_obj(pipeline_obj);
        return NULL;
    }
    if (add_cmd_to_pipeline(pipeline_obj, cmd_obj) == -1) {
        destroy_pipeline_obj(pipeline_obj);
        destroy_cmd_obj(cmd_obj);
        return NULL;
    }

    while (tokens[*current].type == PIPE) {
        *current += 1;

        cmd_obj = parse_primary(tokens, current);
        if (cmd_obj == NULL) {
            destroy_pipeline_obj(pipeline_obj);
            return NULL;
        }
        if (add_cmd_to_pipeline(pipeline_obj, cmd_obj) == -1) {
            destroy_pipeline_obj(pipeline_obj);
            destroy_cmd_obj(cmd_obj);
            return NULL;
        }
    }

    /* Add pipeline to table and store the index 
       at which the pipeline is added in the table */
    int pipeline_index = add_pipeline_to_table(pipeline_table, pipeline_obj);
    if (pipeline_index == -1) {
        destroy_pipeline_obj(pipeline_obj);
        return NULL;
    }

    /* Node of type `PIPELINE` will store index of the pipeline */
    Ast_node *node = create_ast_node(PIPELINE, pipeline_index);
    if (node == NULL) {
        /* don't destroy pipeline object or table, 
            as `parse_sequence` will itself do that */
        return NULL;
    }

    return node;
}


#define GET_ROOT_TYPE(tokens, current) \
        tokens[*current].type == LOGIC_AND \
        ? AND : OR

static Ast_node *
parse_condition(Token *tokens, int *current, Pipeline_table *pipeline_table)
{
    Ast_node *left = parse_pipeline(tokens, current, pipeline_table);
    if (left == NULL) {
        return NULL;
    }

    while (tokens[*current].type == LOGIC_AND
        || tokens[*current].type == LOGIC_OR) {

        /* root_type will be `OR` or `AND` */
        Node_type root_type = GET_ROOT_TYPE(tokens, current);
        *current += 1;

        Ast_node *right = parse_pipeline(tokens, current, pipeline_table);
        if (right == NULL) {
            destroy_ast(left);
            return NULL;
        }

        Ast_node *root = create_ast_node(root_type, -1);
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

#undef GET_ROOT_TYPE


static Parser_obj *
parse_sequence(Token *tokens, int *current)
{
    /* Pipeline table stores all the pipelines */
    Pipeline_table *pipeline_table = get_pipeline_table();
    if (pipeline_table == NULL) {
        return NULL;
    }

    Ast_node *left = parse_condition(tokens, current, pipeline_table);
    if (left == NULL) {
        destroy_pipeline_table(pipeline_table);
        return NULL;
    }

    while (tokens[*current].type == SEMICOLON) {
        *current += 1;

        Ast_node *right = parse_condition(tokens, current, pipeline_table);
        if (right == NULL) {
            destroy_ast(left);
            destroy_pipeline_table(pipeline_table);
            return NULL;
        }

        Ast_node *root = create_ast_node(NEXT, -1);
        if (root == NULL) {
            destroy_ast(right);
            destroy_ast(left);
            destroy_pipeline_table(pipeline_table);
            return NULL;
        }

        root->left  = left;
        root->right = right;
        left = root;
    }    

    Parser_obj *parser_obj = create_parser_obj(left, pipeline_table);
    if (parser_obj == NULL) {
        destroy_ast(left);
        destroy_pipeline_table(pipeline_table);
        return NULL;
    }

    return parser_obj;
}


Parser_obj *
parse_tokens(Token *tokens)
{
    int current = 0;
    Parser_obj *parser_obj = parse_sequence(tokens, &current);
    if (parser_obj == NULL) {
        return NULL;
    }

    /* Current token type is not `NIL` means that
       we haven't parsed all the tokens */
    if (tokens[current].type != NIL) {
        fprintf(stderr, "Syntax Error\n");
        destroy_parser_obj(parser_obj);
        return NULL;
    }

    return parser_obj;
}
