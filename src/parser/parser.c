#include <stddef.h>

#include "parser.h"
#include "ast.h"
#include "parser_helper.h"
#include "pipeline.h"
#include "token.h"



static Ast_node *parse_list(Token *tokens, int *current, 
                            Pipeline_table *pipe_table);

static Ast_node *parse_condition(Token *tokens, int *current,
                                Pipeline_table *pipe_table);



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

        Node_type root_type = get_node_type(tokens, current);
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

        Node_type root_type = get_node_type(tokens, current);
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
