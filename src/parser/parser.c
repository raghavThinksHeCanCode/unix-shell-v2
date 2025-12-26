#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "ast.h"
#include "token.h"
#include "pipeline.h"
#include "command.h"
#include "parser_helper.h"


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
