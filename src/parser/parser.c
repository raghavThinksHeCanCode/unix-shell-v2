#include <stddef.h>
#include <stdio.h>

#include "parser.h"
#include "token.h"


int
parse_primary(Token *tokens, size_t *current) {
    Token_type curr_tok_type = tokens[*current].type;

    switch (curr_tok_type) {
        case CMD:
            *current += 1;
            curr_tok_type = tokens[*current].type;
            while (curr_tok_type == ARG) {
                *current += 1;
                curr_tok_type = tokens[*current].type;
            }

            return 0;

        default:
            // printf("Invalid Syntax...\n");
            return -1;
    }
}


int
parse_job(Token *tokens, size_t *current)
{
    if (parse_primary(tokens, current) == -1) {
        return -1;
    }

    while (tokens[*current].type == PIPE) {
        *current += 1;
        if (parse_primary(tokens, current) == -1) {
            return -1;
        }
    }

    return 0;
}


int
parse_seq(Token *tokens, size_t *current)
{
    if (parse_job(tokens, current) == -1) {
        return -1;
    }

    while (tokens[*current].type == LOGIC_AND
        || tokens[*current].type == LOGIC_OR) {

        *current += 1;
        if (parse_job(tokens, current) == -1) {
            return -1;
        }
    }

    return 0;
}


int
parse_expr(Token *tokens, size_t *current)
{
    if (parse_seq(tokens, current) == -1) {
        return -1;
    }

    while (tokens[*current].type == SEMICOLON) {
        *current += 1;
        if (parse_seq(tokens, current) == -1) {
            return -1;
        }
    }

    return 0;
}


int
parse_tokens(Token *tokens)
{
    size_t current = 0;
    if (parse_expr(tokens, &current) == -1) {
        return -1;
    }

    if (tokens[current].type != NIL) {
        // printf("Invalid Syntax...\n");
        return -1;
    } else {
        // printf("Valid Syntax...\n");
        return 0;
    }
}
