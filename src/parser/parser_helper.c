#include <stdio.h>
#include <stdlib.h>

#include "parser_helper.h"
#include "ast.h"
#include "parser.h"
#include "pipeline.h"


Parser_obj *
create_parser_obj(Ast_node *ast_root, Pipeline_table *pipeline_table)
{
    Parser_obj *parser_obj = malloc(sizeof(*parser_obj));
    if (parser_obj == NULL) {
        perror("Creating parser object");
        return NULL;
    }

    parser_obj->ast_root       = ast_root;
    parser_obj->pipeline_table = pipeline_table;
    return parser_obj;
}


void
destroy_parser_obj(Parser_obj *parser_obj)
{
    destroy_ast(parser_obj->ast_root);
    destroy_pipeline_table(parser_obj->pipeline_table);
}
