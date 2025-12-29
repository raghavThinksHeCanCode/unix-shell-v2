#ifndef TOKENS_H_
#define TOKENS_H_


#define MAX_LEXEME_SIZE 128
#define MAX_TOK_COUNT   128


typedef enum Token_type
{
    NAME,

    SEMICOLON, AMPERSND,          /* `;` and `&` */

    PIPE,                         /* `|` */
    BACKSLASH,                    /* `\` */

    DOUBLE_AMPERSND, DOUBLE_PIPE, /* `&&` and `||` */

    GREAT, LESS,                  /* `>` and `<` */
    GREAT_AMPERSND,               /* `>&` */
    LESS_AMPERSND,                /* `<&` */
    DOUBLE_GREAT, DOUBLE_LESS,    /* `>>` and `<<` */
    DOUBLE_GREAT_AMPERSND,        /* `>>&` */

    NIL                           /* To represent last token */
} Token_type;


typedef struct Token
{
    Token_type type;
    char      *lexeme;
} Token;


#endif
