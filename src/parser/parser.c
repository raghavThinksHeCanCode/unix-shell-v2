#include <stdio.h>
#include <stddef.h>

#include "parser.h"
#include "token.h"


// static int parse_expr(Token *tokens, size_t *current);
static int parse_seq(Token *tokens, size_t *current);
static int parse_job(Token *tokens, size_t *current);
static int parse_pipeline(Token *tokens, size_t *current);
static int parse_proc(Token *tokens, size_t *current);


static int
parse_proc(Token *tokens, size_t *current)
{
    Token_type curr_token_type = tokens[*current].type;

    switch (curr_token_type) {
        case NAME:
            while (curr_token_type == NAME) {
                *current += 1;
                curr_token_type = tokens[*current].type;
            }
            return 0;

        case LEFT_PAREN:
            //handle left parenthesis

        default:
            fprintf(stderr, "Syntax error...\n");
            return -1;
    }
}


static int
parse_pipeline(Token *tokens, size_t *current)
{
    int err_return = parse_proc(tokens, current);
    if (err_return == -1) {
        return -1;
    }

    while (tokens[*current].type == PIPE) {
        *current += 1;
        err_return = parse_proc(tokens, current);
        if (err_return == -1) {
            return -1;
        }
    }

    return 0;
}


static int
parse_job(Token *tokens, size_t *current)
{
    int err_return = parse_pipeline(tokens, current);
    if (err_return == -1) {
        return -1;
    }

    while (tokens[*current].type == LEFT_REDIR
        || tokens[*current].type == RIGHT_REDIR
        || tokens[*current].type == DOUBLE_RIGHT_REDIR) {

        *current += 1;

        if (tokens[*current].type != NAME) {
            return -1;
        }

        *current += 1;
    }

    return 0;
}


static int
parse_seq(Token *tokens, size_t *current)
{
    int err_return = parse_job(tokens, current);
    if (err_return == -1) {
        return -1;
    }

    while (tokens[*current].type == LOGIC_AND || tokens[*current].type == LOGIC_OR
        || tokens[*current].type == SEMICOLON) {

        /* Consume the token */
        *current += 1;

        err_return = parse_job(tokens, current);
        if (err_return == -1) {
            return -1;
        }
    }

    return 0;
}


int
parse_tokens(Token *tokens)
{
    size_t current = 0;

    int err_return = parse_seq(tokens, &current);
    if (err_return == -1) {
        return -1;
    }

    /* If current token type is not `NIL` after
       even after parsing all the tokens,
       it means a syntax error has occured */
    if (tokens[current].type != NIL) {
        fprintf(stderr, "Syntax error...\n");
        return -1;
    }

    return 0;
}
