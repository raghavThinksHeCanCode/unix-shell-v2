#include <stdio.h>

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
