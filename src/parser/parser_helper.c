#include <stdio.h>
#include <stdlib.h>

#include "parser_helper.h"
#include "ast.h"
#include "parser.h"
#include "pipeline.h"
#include "token.h"


Parser_obj *
create_parser_obj(Ast_node *ast_root, Pipeline_table *pipe_table)
{
    Parser_obj *parser_obj = malloc(sizeof(*parser_obj));
    if (parser_obj == NULL) {
        perror("create_parser_obj");
        return NULL;
    }

    parser_obj->ast_root   = ast_root;
    parser_obj->pipe_table = pipe_table;
    return parser_obj;
}


void
destroy_parser_obj(Parser_obj *parser_obj)
{
    destroy_ast(parser_obj->ast_root);
    destroy_pipeline_table(parser_obj->pipe_table);
}


Node_type
get_node_type(Token_type token_type)
{
    switch (token_type) {
        case DOUBLE_AMPRSND:
            return AND;

        case DOUBLE_PIPE:
            return OR;

        case SEMICLN:
            return NEXT;

        case AMPRSND:
            return BG_NEXT;
    }
}
