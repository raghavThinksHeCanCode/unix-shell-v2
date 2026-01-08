#ifndef LEXER_H_
#define LEXER_H_

#include "token.h"


/* Tokenize input by returning `Token`
   array. Return `NULL` on error */
Token *tokenize(const char *input);


#endif
