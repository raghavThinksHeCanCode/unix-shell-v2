#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "parser_helper.h"
#include "token.h"


static char *
identify_lexeme(Token *token)
{
    Token_type token_type = token->type;
    char *lexeme = NULL;

    switch(token_type) {
        case PIPE:
            lexeme = "|";
            break;

        case BG_OPERATOR:
            lexeme = "&";
            break;

        case LOGIC_AND:
            lexeme = "&&";
            break;

        case LOGIC_OR:
            lexeme = "||";
            break;

        case SEMICOLON:
            lexeme = ";";
            break;

        case LEFT_REDIR:
            lexeme = "<";
            break;

        case RIGHT_REDIR:
            lexeme = ">";
            break;

        case DOUBLE_RIGHT_REDIR:
            lexeme = ">>";
            break;

        case LEFT_PAREN:
            lexeme = "(";
            break;

        case RIGHT_PAREN:
            lexeme = ")";
            break;

        case NAME:
            lexeme = token->lexeme;
            break;

        default:
            /* dummy case to silence compiler warning */
            break;
    }

    return lexeme;
}


void
print_err_msg(const char *msg, Token *token)
{
    if (!token) {
        fprintf(stderr, "%s", msg);
    }

    char *lexeme = identify_lexeme(token);
    fprintf(stderr, "%s '%s'\n", msg, lexeme);
}


int
push_paren(Paren_stack *paren_stack)
{
    Paren_stack *new = malloc(sizeof(*new));
    if (new == NULL) {
        perror("push_paren");
        return -1;
    }

    new->previous = paren_stack;
    paren_stack   = new;

    return 0;
}


void
pop_paren(Paren_stack *paren_stack)
{
    assert(paren_stack != NULL);
    Paren_stack *temp = paren_stack->previous;
    free(paren_stack);
    paren_stack = temp;
}
