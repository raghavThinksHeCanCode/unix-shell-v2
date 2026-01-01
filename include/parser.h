#ifndef PARSER_H_
#define PARSER_H_


#include "token.h"
#include "ast.h"
#include "pipeline.h"


typedef struct Parser_obj
{
    Pipeline_table *pipe_table;
    Ast_node       *ast_root;
} Parser_obj;


/*
    Returns pointer to `Parser_obj` whose members 
    are a pointer to  `Ast_node`, and a pointer to 
    `Pipeline_table`. Returns `NULL` on error.
*/
Parser_obj *parse_tokens(Token *tokens);


#endif // PARSER_H_
