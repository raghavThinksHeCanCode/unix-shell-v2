#include <stdio.h>
#include <stddef.h>

#include "parser.h"
#include "parser_helper.h"
#include "token.h"


Paren_stack *paren_stack = NULL;


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
                consume_token(current);
                curr_token_type = tokens[*current].type;
            }
            return 0;

        case LEFT_PAREN:
            /* Push another item in parenthesis stack */
            if (push_paren(paren_stack) == -1) {
                return -1;
            }
            consume_token(current);

            int err_return = parse_seq(tokens, current);
            if (err_return == -1) {
                return -1;
            }
            return 0;

        default:
            print_err_msg("A syntax error has occured near", &tokens[*current]);
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
        consume_token(current);

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

        consume_token(current);

        if (tokens[*current].type != NAME) {
            return -1;
        }
        consume_token(current);
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

        consume_token(current);

        err_return = parse_job(tokens, current);
        if (err_return == -1) {
            return -1;
        }
    }

    if (tokens[*current].type == RIGHT_PAREN) {
        if (!paren_stack) {
            /* Current token is right parenthesis but
               paren_stack is empty. This means there is
               an extra right parenthesis */
            print_err_msg("Syntax Error: Unmatched )", NULL);
            return -1;
        }

        pop_paren(paren_stack);
        return 0;
    }

    /* If current token type is not right parenthesis
       but the parenthesis stack is not empty. This means
       that we haven't passed a closing parenthesis */
    if (paren_stack) {
        print_err_msg("Syntax Error: ( was never closed", NULL);
        return -1;
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

    if (tokens[current].type != NIL) {
        /* If current token type is not `NIL`
        even after parsing all the tokens,
        it means a syntax error has occured */

        print_err_msg("A syntax error has occured near", &tokens[current]);
        return -1;
    }

    return 0;
}
