#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer_helper.h"
#include "utils.h"


struct Lexer_obj *
get_lexer_obj(const char *input)
{
    struct Lexer_obj *lexer_obj = malloc(sizeof(*lexer_obj));
    if (lexer_obj == NULL) {
        perror(NULL);
        return NULL;
    }

    lexer_obj->tokens    = NULL;
    lexer_obj->tok_count = 0;
    lexer_obj->source    = input;
    lexer_obj->start     = 0;
    lexer_obj->current   = 0;

    return lexer_obj;
}


bool
lex_current_at_end(struct Lexer_obj *lexer_obj)
{
    const char *source = lexer_obj->source;
    size_t current     = lexer_obj->current;

    if (source[current] == '\0') {
        return true;
    }
    return false;
}


/* Advances current and returns the character
    previously pointed by current */
char
lex_advance_current(struct Lexer_obj *lexer_obj)
{
    lexer_obj->current += 1;
    size_t current      = lexer_obj->current;
    const char *source  = lexer_obj->source;

    return source[current - 1];
}


bool
lex_peek(struct Lexer_obj *lexer_obj, const char expected)
{
    const char *source = lexer_obj->source;
    size_t current     = lexer_obj->current;

    if (source[current] == expected) return true;

    return false;
}


void
lex_init_token(Token *token, Token_type type)
{
    token->type   = type;
    token->lexeme = NULL;
}


char *
create_substring(const char *string, size_t start, size_t end)
{
    /* Add `1` for null-byte */
    size_t buf_size = (end - start) + 1;
    char *substring = malloc(buf_size * sizeof(*substring));

    if (!substring) return NULL;

    string += start;  /* `string` ptr points to start of substring */
    memcpy(substring, string, buf_size - 1);
    substring[buf_size - 1] = '\0';

    return substring;
}


void
destroy_lex_data(struct Lexer_obj *lexer_obj)
{
    Token *tokens       = lexer_obj->tokens;
    size_t tokens_count = lexer_obj->tok_count - 1;

    free(lexer_obj);
    free_tokens(tokens, tokens_count);
}


int
lex_expand_tok_array(struct Lexer_obj *lexer_obj)
{
    Token *tokens = lexer_obj->tokens;
    Token *temp   = NULL;

    /*
        See this SO answer for realloc error handling:
        https://stackoverflow.com/a/1986572/31078065
     */
    size_t new_arr_size = lexer_obj->tok_count + 1;
    temp = realloc(tokens, new_arr_size * sizeof(*tokens));

    if (!temp) {
        perror(NULL);
        free(tokens);
        return -1;
    }

    lexer_obj->tokens    = temp;
    lexer_obj->tok_count = new_arr_size;
    return 0;
}