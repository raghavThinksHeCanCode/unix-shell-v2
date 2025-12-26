#ifndef PARSER_H_
#define PARSER_H_


#include "token.h"
#include "ast.h"
#include "pipeline.h"


typedef struct Parser_obj
{
    Pipeline_table *pipeline_table;
    Ast_node       *ast_root;
} Parser_obj;


Parser_obj *parse_tokens(Token *tokens);


#endif // PARSER_H_
