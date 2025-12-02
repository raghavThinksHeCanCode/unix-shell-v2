#ifndef PARSER_HELPER_
#define PARSER_HELPER_


#include <stddef.h>

#include "token.h"


void print_err_msg(const char *msg, Token *token);
void consume_token(size_t *current);


#endif // PARSER_HELPER_
