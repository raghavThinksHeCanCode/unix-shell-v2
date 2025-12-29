#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "token.h"
#include "lexer_helper.h"


static int add_word(struct Lexer_obj *lexer_obj);
static int add_token(struct Lexer_obj *lexer_obj, Token_type type);
static int handle_word(struct Lexer_obj *lexer_obj);
static int scan_token(struct Lexer_obj *lexer_obj);


/*
    @brief : Adds literal lexeme in the token for token of 
    type `NAME` which represents the name of command, file
    or argument

    @return: -1 on failure; 0 on success
*/
static int
add_word(struct Lexer_obj *lexer_obj)
{
    const char *string = lexer_obj->source;
    size_t start       = lexer_obj->start;
    size_t end         = lexer_obj->current;
    size_t cur_index   = lexer_obj->tok_count - 1;

    char *substring = create_substring(string, start, end);

    if (substring == NULL) {
        return -1;
    }

    lexer_obj->tokens[cur_index].lexeme = substring;
    return 0;
}


/*
    @brief : Adds another token into the `tokens` array of
    `Lexer_obj`

    @param : Pointer to `struct Lexer_obj` and a `Token_type`
    variable `type` which represents the type of token to add

    @return: -1 on failure; 0 on success
*/
static int
add_token(struct Lexer_obj *lexer_obj, Token_type type)
{
    if (lexer_obj->tok_count == MAX_TOK_COUNT) {
        /* If already at max allowed tokens count */
        return -1;
    }

    /* Expand the tokens array to add space for new token */
    if (lex_expand_tok_array(lexer_obj) == -1) {
        return -1;
    }

    Token *tokens    = lexer_obj->tokens;
    size_t cur_index = lexer_obj->tok_count - 1;
    Token *cur_token = &tokens[cur_index];

    lex_init_token(cur_token, type);

    if (type == NAME) {
        /* Add lexeme value for token of type `NAME` */
        if (add_word(lexer_obj) == -1) return -1;
    }

    return 0;
}


/*
    @brief : Tokenize a lexeme of type NAME which represents either a command,
    its arguments or a filename

    @return: -1 on failure; 0 on success
*/
static int
handle_word(struct Lexer_obj *lexer_obj)
{
    /* Move current ahead, until any of the recognised lexeme is not found */
    while (!lex_peek(lexer_obj, ' ') && !lex_peek(lexer_obj, '\t')
            && !lex_peek(lexer_obj, '\0') && !lex_peek(lexer_obj, ';')
            && !lex_peek(lexer_obj, '&') && !lex_peek(lexer_obj, '|')
            && !lex_peek(lexer_obj, '>') && !lex_peek(lexer_obj, '<')
            && !lex_peek(lexer_obj, '(') && !lex_peek(lexer_obj, ')')) {

        lex_advance_current(lexer_obj);

        const size_t curr_lexeme_size = 
            find_curr_lexeme_size(lexer_obj->start, lexer_obj->current);

        if (curr_lexeme_size > MAX_LEXEME_SIZE) {
            fprintf(stderr, "Maximum allowed lexeme size exceeded\n");
            return -1;
        }
    }

    int err_return = add_token(lexer_obj, NAME);
    return err_return;
}


/*
    @brief : Identifies the current lexeme type
    @return: 0 on success; -1 on failure
*/
static int
scan_token(struct Lexer_obj *lexer_obj)
{
    /* Get the first character of current lexeme */
    const char c   = lex_advance_current(lexer_obj);
    int err_return = 0;

    switch (c) {
        /* == Whitespaces == */
        case ' ': case '\t':
            /* Skip whitespaces */
            break;


        /* == `;` == */
        case ';':
            err_return = add_token(lexer_obj, SEMICLN);
            break;


        /* == `\` == */
        case '\\':  /* Using esc-seq to represent `\` */
            err_return = add_token(lexer_obj, BACKSLSH);
            break;


        /* == `|` and `||` == */
        case '|':
            if ('|' == lex_get_curr_char(lexer_obj)) {
                lex_advance_current(lexer_obj);
                err_return = add_token(lexer_obj, DOUBLE_PIPE);
            }
            else {
                err_return = add_token(lexer_obj, PIPE);
            }
            break;


        /* == `&` and `&&` == */
        case '&':
            if ('&' == lex_get_curr_char(lexer_obj)) {
                lex_advance_current(lexer_obj);
                err_return = add_token(lexer_obj, DOUBLE_AMPRSND);
            }
            else {
                err_return = add_token(lexer_obj, AMPRSND);
            }
            break;


        /* == `<` and `<&` == */
        case '<':
            /* `<&` and `<` */
            if ('&' == lex_get_curr_char(lexer_obj)) {
                lex_advance_current(lexer_obj);
                err_return = add_token(lexer_obj, LESS_AMPRSND);
            }
            else {
                err_return = add_token(lexer_obj, LESS);
            }
            break;


        /* == `>` `>>` `>>&` and `>&` == */
        case '>':
            ;
            char curr_ch = lex_get_curr_char(lexer_obj);

            if (curr_ch == '>') {
                /* Handle `>>` and `>>&` */
                lex_advance_current(lexer_obj);
                curr_ch = lex_get_curr_char(lexer_obj);

                if (curr_ch == '&') {
                    lex_advance_current(lexer_obj);
                    err_return = add_token(lexer_obj, DOUBLE_GREAT_AMPRSND);
                }
                else {
                    err_return = add_token(lexer_obj, DOUBLE_GREAT);
                }
            }
            else if (curr_ch == '&') {
                /* Handle `>&` */
                lex_advance_current(lexer_obj);
                err_return = add_token(lexer_obj, GREAT_AMPRSND);
            }
            else {
                /* Hanlde `>` */
                err_return = add_token(lexer_obj, GREAT);
            }
            break;


        /* ==== Word tokens ==== */
        default:
            err_return = handle_word(lexer_obj);
            break;
    }

   return err_return; /* -1 on failure; 0 on success */
}


/*
    @brief: Main tokenizer interface. Tokenizes a given Null-terminated string into
    lexical tokens.

    @return: A pointer to an array of type `Token`. The caller is responsible for freeing
    the array. `NULL` on error
*/
Token *
tokenize(const char *input)
{
    struct Lexer_obj *lexer_obj = lex_init_obj(input);
    if (lexer_obj == NULL) {
        return NULL;
    }

    int err_return = 0;

    /* ==== Main tokenizer loop ==== */

    while (lex_current_at_end(lexer_obj) == false) {
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

    /* Only free the unnecessary lexer data like
       current and start position.*/
    Token *tokens = lexer_obj->tokens;
    free(lexer_obj);

    return tokens;
}
