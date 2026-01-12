#ifndef LEXER_HELPER_H_
#define LEXER_HELPER_H_


#include <stdlib.h>
#include <stdbool.h>

#include "token.h"


struct Lexer_obj
{
    Token *tokens;    /* Pointer to the current generated tokens */
    size_t tok_count; /* Size of the `tokens` array */

    const char *source;    /* string to be tokenized */
    size_t      start;     /* 1st char of the lexeme */
    size_t      current;   /* current char of string being considered */
};


#define LEX_GET_CURR_CHAR(lexer_obj) \
        lexer_obj->source[lexer_obj->current] \

#define CURR_LEXEME_SIZE(lexer_obj) \
        lexer_obj->current - lexer_obj->start \

struct Lexer_obj *get_lexer_obj(const char *input);
bool lex_current_at_end(struct Lexer_obj *lexer_obj);
char lex_advance_current(struct Lexer_obj *lexer_obj);
void lex_init_token(Token *token, Token_type type);
int lex_expand_tok_array(struct Lexer_obj *lexer_obj);
char *create_substring(const char *string, size_t start, size_t end);
void destroy_lexer_obj(struct Lexer_obj *lexer_obj);


#endif // LEXER_HELPER_
