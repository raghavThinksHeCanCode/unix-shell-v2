#ifndef PARSER_HELPER_
#define PARSER_HELPER_


#include <stddef.h>

#include "token.h"


typedef struct Paren_stack
{
    struct Paren_stack *previous;
} Paren_stack;


void print_err_msg(const char *msg, Token *token);
void consume_token(size_t *current);
int push_paren(Paren_stack *paren_stack);
void pop_paren(Paren_stack *paren_stack);


#endif // PARSER_HELPER_
