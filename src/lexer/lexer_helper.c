#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer_helper.h"
#include "token.h"
#include "utils.h"


Lexer_obj *
get_lexer_obj(const char *input)
{
    Lexer_obj *lexer_obj = malloc(sizeof(*lexer_obj));
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


/* Advances current and returns the character
    previously pointed by current */
char
advance_current(Lexer_obj *lexer_obj)
{
    lexer_obj->current += 1;
    int current      = lexer_obj->current;
    const char *source  = lexer_obj->source;

    return source[current - 1];
}


bool
is_current_at_end(Lexer_obj *lexer_obj)
{
    const char *string = lexer_obj->source;
    const int current = lexer_obj->current;

    if (string[current] == '\0' || string[current] == '\n') {
        return true;
    }

    return false;
}


void
init_token(Token *token, Token_type type)
{
    token->type   = type;
    token->lexeme = NULL;
}


char *
create_substring(const char *string, int start, int end)
{
    /* Add `1` for null-byte */
    int buf_size = (end - start) + 1;
    char *substring = malloc(buf_size * sizeof(*substring));

    if (substring == NULL) { 
        return NULL;
    }

    string += start;  /* `string` ptr points to start of substring */
    memcpy(substring, string, buf_size - 1);
    substring[buf_size - 1] = '\0';

    return substring;
}


void
destroy_lexer_obj(Lexer_obj *lexer_obj)
{
    Token *tokens       = lexer_obj->tokens;
    int tokens_count = lexer_obj->tok_count - 1;

    for (int i = 0; i < tokens_count; i++) {
        if (tokens[i].type == NAME) {
            free(tokens[i].lexeme);
        }
    }

    free(tokens);
    free(lexer_obj);
}


int
expand_tok_array(Lexer_obj *lexer_obj)
{
    Token *tokens = lexer_obj->tokens;
    Token *temp   = NULL;

    /*
        See this SO answer for realloc error handling:
        https://stackoverflow.com/a/1986572/31078065
     */
    int new_arr_size = lexer_obj->tok_count + 1;
    temp = realloc(tokens, new_arr_size * sizeof(*temp));

    if (temp == NULL) {
        perror("lex_expand_tok_array");
        free(tokens);
        return -1;
    }

    lexer_obj->tokens    = temp;
    lexer_obj->tok_count = new_arr_size;
    return 0;
}