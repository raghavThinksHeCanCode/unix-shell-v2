#ifndef PARSER_HELPER_
#define PARSER_HELPER_


#include "ast.h"
#include "parser.h"
#include "pipeline.h"


Parser_obj *create_parser_obj(Ast_node *ast_root, Pipeline_table *pipeline_table);
void destroy_parser_obj(Parser_obj *parser_obj);


#endif // PARSER_HELPER_
