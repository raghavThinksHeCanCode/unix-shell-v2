#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "token.h"
#include "lexer_helper.h"


static int add_arg(struct Lexer_obj *lexer_obj);
static int add_token(struct Lexer_obj *lexer_obj, Token_type type);
static int handle_command(struct Lexer_obj *lexer_obj, Token_type type);
static int scan_token(struct Lexer_obj *lexer_obj);


static int
add_arg(struct Lexer_obj *lexer_obj)
{
    char  *string    = lexer_obj->source;
    size_t start     = lexer_obj->start;
    size_t end       = lexer_obj->current;
    size_t cur_index = lexer_obj->tok_count - 1;

    char *substring = create_substring(string, start, end);

    if (substring == NULL) return -1;

    lexer_obj->tokens[cur_index].arg = substring;
    return 0;
}


static int
add_token(struct Lexer_obj *lexer_obj, Token_type type)
{
    if (lex_expand_tok_array(lexer_obj) == -1) return -1;

    Token *tokens    = lexer_obj->tokens;
    size_t cur_index = lexer_obj->tok_count - 1;
    Token *cur_token = &tokens[cur_index];

    lex_init_token(cur_token, type);

    if (type == CMD || type == ARG) {
        if (add_arg(lexer_obj) == -1) return -1;
    }

    return 0;
}


static int
handle_command(struct Lexer_obj *lexer_obj, Token_type type)
{
    /* Move current ahead, until any of the recognised lexeme is not found */
    while (!lex_peek(lexer_obj, ' ') && !lex_peek(lexer_obj, '\t')
            && !lex_peek(lexer_obj, '\0') && !lex_peek(lexer_obj, ';')
            && !lex_peek(lexer_obj, '&') && !lex_peek(lexer_obj, '|')
            && !lex_peek(lexer_obj, '>') && !lex_peek(lexer_obj, '<')
            && !lex_peek(lexer_obj, '(') && !lex_peek(lexer_obj, ')')) {

        lex_advance_current(lexer_obj);
    }

    int err_return = add_token(lexer_obj, type);
    return err_return;
}


static int
scan_token(struct Lexer_obj *lexer_obj)
{
    char c = lex_advance_current(lexer_obj);
    size_t tok_count;   /* Used inside the default case */
    int err_return = 0;

    switch (c) {
        /* === Single character tokens === */

        case ' ': case '\t':
            /* Skip whitespaces */
            break;

        case ';':
            err_return = add_token(lexer_obj, SEMICOLON);
            break;

        case '(':
            err_return = add_token(lexer_obj, LEFT_PAREN);
            break;

        case ')':
            err_return = add_token(lexer_obj, RIGHT_PAREN);
            break;

        /* There is no `<<` token for the shell */
        case '<':
            err_return = add_token(lexer_obj, LEFT_REDIR);
            break;

        /* === Double/single character tokens === */
        /*
            For each of these cases, peek the next character
            in the string. If the current and the next character
            are the same, then it's a double character token. Else
            single character token.
        */

        case '|':
            if (lex_peek(lexer_obj, '|') == true) {
                lex_advance_current(lexer_obj);
                err_return = add_token(lexer_obj, LOGIC_OR);
            }
            else {
                err_return = add_token(lexer_obj, PIPE);
            }
            break;

        case '&':
            if (lex_peek(lexer_obj, '&')) {
                lex_advance_current(lexer_obj);
                err_return = add_token(lexer_obj, LOGIC_AND);
            }
            else {
                err_return = add_token(lexer_obj, BG_OPERATOR);
            }
            break;

        case '>':
            if (lex_peek(lexer_obj, '>')) {
                lex_advance_current(lexer_obj);
                err_return = add_token(lexer_obj, DOUBLE_RIGHT_REDIR);
            }
            else {
                err_return = add_token(lexer_obj, RIGHT_REDIR);
            }
            break;

        /* === Command tokens === */

        default:
            tok_count                     = lexer_obj->tok_count;
            Token_type type_of_last_token = lexer_obj->tokens[tok_count - 1].type;

            /*
                If the type of last token is either `CMD` or `ARG`, then the
                current lexeme type is `ARG`. Otherwise, its `CMD`.
                If there are 0 tokens in the array, then also current
                lexeme type is `CMD`
            */
            if (tok_count > 0 
                && (type_of_last_token == CMD || type_of_last_token == ARG)) {
                err_return = handle_command(lexer_obj, ARG);
            }
            else {
                err_return = handle_command(lexer_obj, CMD);
            }
            break;

    }

   return err_return;
}


Token *
tokenize(char *input)
{
    /* Create lexer object */
    struct Lexer_obj *lexer_obj = malloc(sizeof(*lexer_obj));

    if (lexer_obj == NULL) {
        perror(NULL);
        return NULL;
    }

    lex_init_obj(lexer_obj, input); /* Initialize lexer object */
    int err_return = 0;

    /* Main tokenizing loop */
    while (!lex_current_at_end(lexer_obj)) {
        /* Move to the next lexeme */
        lexer_obj->start = lexer_obj->current;
        err_return       = scan_token(lexer_obj);

        if (err_return == -1) {
            destroy_lex_data(lexer_obj);
            return NULL;
        }
    }

    /* Add `NIL` as last token */
    err_return = add_token(lexer_obj, NIL);

    if (err_return == -1) {
        destroy_lex_data(lexer_obj);
        return NULL;
    }

    Token *tokens = lexer_obj->tokens;
    free(lexer_obj);

    return tokens;
}
