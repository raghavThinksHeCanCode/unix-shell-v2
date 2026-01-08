#ifndef TOKENS_H_
#define TOKENS_H_


#define MAX_LEXEME_SIZE 128
#define MAX_TOK_COUNT   128


typedef enum Token_type
{
    NAME,

    SEMICLN, AMPRSND,            /* `;` and `&` */

    PIPE,                        /* `|` */
    BACKSLSH,                    /* `\` */

    DOUBLE_AMPRSND, DOUBLE_PIPE, /* `&&` and `||` */

    GREAT, LESS,                 /* `>` and `<` */
    GREAT_AMPRSND,               /* `>&` */
    LESS_AMPRSND,                /* `<&` */
    DOUBLE_GREAT,                /* `>>` */
    DOUBLE_GREAT_AMPRSND,        /* `>>&` */

    NIL                           /* To represent last token */
} Token_type;


typedef struct Token
{
    Token_type type;
    char      *lexeme; /* string value for NAME */
} Token;


#endif
