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


#define GET_NODE_TYPE(tokens, current) \
        tokens[*current].type == SEMICLN \
        ? NEXT : BG_NEXT

static Ast_node *
parse_list(Token *tokens, int *current, Pipeline_table *pipe_table)
{

    Ast_node *left = parse_condition(tokens, current, pipe_table);
    if (left == NULL) {
        return NULL;
    }

    while (tokens[*current].type == SEMICLN
        || tokens[*current].type == AMPRSND) {

        Node_type root_type = GET_NODE_TYPE(tokens, current);
        *current += 1;

        Ast_node *root = create_ast_node(root_type, -1);
        if (root == NULL) {
            destroy_ast(left);
            return NULL;
        }

        root->left = left;

        if (tokens[*current].type != NIL) {
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

#undef GET_NODE_TYPE


Parser_obj *
parse_tokens(Token *tokens)
{
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
