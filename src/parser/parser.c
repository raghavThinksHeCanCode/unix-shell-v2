#include <stddef.h>
#include <stdio.h>

#include "parser.h"
#include "ast.h"
#include "command.h"
#include "parser_helper.h"
#include "pipeline.h"
#include "token.h"



static Command_obj *parse_command(Token *tokens, int *current);

static Ast_node *parse_pipeline(Token *tokens, int *current,
                                Pipeline_table *pipe_table);

static Ast_node *parse_condition(Token *tokens, int *current,
                                Pipeline_table *pipe_table);

static Ast_node *parse_list(Token *tokens, int *current, 
                            Pipeline_table *pipe_table);



static Command_obj *
parse_command(Token *tokens, int *current)
{
    /* Syntax error if current token type is not a NAME */
    if (tokens[*current].type != NAME) {
        fprintf(stderr, "Syntax Error\n");
        return NULL;
    }

    /*
        Add args to command obj and return that
    */
    Command_obj *cmd_obj = get_cmd_obj();
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

    return cmd_obj;
}



static Ast_node *
parse_pipeline(Token *tokens, int *current, Pipeline_table *pipe_table)
{
    /*
        Add command objs to pipeline obj and finally
        add pipeline obj to pipeline table 
    */
    Pipeline_obj *pipe_obj = get_pipeline_obj();
    if (pipe_obj == NULL) {
        return NULL;
    }

    Command_obj *cmd_obj = parse_command(tokens, current);
    if (cmd_obj == NULL) {
        destroy_pipeline_obj(pipe_obj);
        return NULL;
    }

    if (add_cmd_to_pipeline(pipe_obj, cmd_obj) == -1) {
        destroy_pipeline_obj(pipe_obj);
        destroy_cmd_obj(cmd_obj);
        return NULL;
    }

    while (tokens[*current].type == PIPE) {
        *current += 1;

        cmd_obj = parse_command(tokens, current);
        if (cmd_obj == NULL) {
            destroy_pipeline_obj(pipe_obj);
            return NULL;
        }

        if (add_cmd_to_pipeline(pipe_obj, cmd_obj) == -1) {
            destroy_pipeline_obj(pipe_obj);
            destroy_cmd_obj(cmd_obj);
            return NULL;
        }
    }

    int pipeline_index = add_pipeline_to_table(pipe_table, pipe_obj);
    if (pipeline_index == -1) {
        destroy_pipeline_obj(pipe_obj);
        return NULL;
    }

    /* Pipeline node with index being the index 
       in pipeline table where pipeline is stored*/
    Ast_node *node = create_ast_node(PIPELINE, pipeline_index);
    if (node == NULL) {
        /* Caller will destroy pipeline table */
        return NULL;
    }

    return node;
}



static Ast_node *
parse_condition(Token *tokens, int *current, Pipeline_table *pipe_table)
{
    /*
        Just like a list, get right and left child and root 
        being of type `AND` or `OR`.
    */
    Ast_node *left = parse_pipeline(tokens, current, pipe_table);
    if (left == NULL) {
        return NULL;
    }

    while (tokens[*current].type == DOUBLE_AMPRSND
        || tokens[*current].type == DOUBLE_PIPE) {

        Node_type root_type = get_node_type(tokens[*current].type);
        *current += 1;

        Ast_node *root = create_ast_node(root_type, -1);
        if (root == NULL) {
            destroy_ast(left);
            return NULL;
        }

        /* Unlike a list, right operand must exist */
        Ast_node *right = parse_pipeline(tokens, current, pipe_table);
        if (right == NULL) {
            destroy_ast(root);
            destroy_ast(left);
            return NULL;
        }

        root->left  = left;
        root->right = right;

        left = root;
    }

    return left;
}



static Ast_node *
parse_list(Token *tokens, int *current, Pipeline_table *pipe_table)
{
    /*
        Get `left` and `right` child of the node with root being the
        type `NEXT` or `BG_NEXT`.
    */

    Ast_node *left = parse_condition(tokens, current, pipe_table);
    if (left == NULL) {
        return NULL;
    }

    while (tokens[*current].type == SEMICLN
        || tokens[*current].type == AMPRSND) {

        Node_type root_type = get_node_type(tokens[*current].type);
        *current += 1;

        Ast_node *root = create_ast_node(root_type, -1);
        if (root == NULL) {
            destroy_ast(left);
            return NULL;
        }

        root->left = left;

        if (tokens[*current].type != NIL) {
            /*
                If right operand of `;` or `&` exists.
                Remember that `;` and `&` can have no
                operand at right.
            */
            Ast_node *right = parse_condition(tokens, current, pipe_table);
            if (right == NULL) {
                destroy_ast(root);
                return NULL;
            }

            root->right = right;
        }

        left = root;
    }

    return left;
}



Parser_obj *
parse_tokens(Token *tokens)
{
    /*
        Create a pipeline table and get Ast root
        from parsing the tokens. Package both into
        Parser_obj and return that.
    */
    int current = 0;
    Pipeline_table *pipe_table = get_pipeline_table();
    if (pipe_table == NULL) {
        return NULL;
    }

    Ast_node *root = parse_list(tokens, &current, pipe_table);
    if (root == NULL) {
        return NULL;
    }

    if (tokens[current].type != NIL) {
        /*
            If `current` didn't reach to the last token,
            this means not all tokens are parsed and thus error.
        */
        destroy_pipeline_table(pipe_table);
        destroy_ast(root);
        return NULL;
    }

    Parser_obj *parser_obj = create_parser_obj(root, pipe_table);
    if (parser_obj == NULL) {
        destroy_pipeline_table(pipe_table);
        destroy_ast(root);
        return NULL;
    }

    return parser_obj;
}
