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
    char curr_ch = lex_get_curr_char(lexer_obj);

    /* Move current ahead, until any of the recognised lexeme is not found */
    while (curr_ch != ' ' && curr_ch != '\t' && curr_ch != '\0'
        && curr_ch != ';' && curr_ch != '&'  && curr_ch != '|'
        && curr_ch != '>' && curr_ch != '<') {

        lex_advance_current(lexer_obj);
        curr_ch = lex_get_curr_char(lexer_obj);

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


/* Identify each token in the string */
static int
scan_token(struct Lexer_obj *lexer_obj)
{
    /* Get the first character of current lexeme */
    const char c   = lex_advance_current(lexer_obj);
    int err_return = 0;

    switch (c) {
        case ' ': case '\t':
            /* Skip whitespaces */
            break;

        case ';':
            err_return = add_token(lexer_obj, SEMICLN);
            break;

        case '\\':  /* Match `\` */
            err_return = add_token(lexer_obj, BACKSLSH);
            break;

        case '|':
            if ('|' == lex_get_curr_char(lexer_obj)) {
                lex_advance_current(lexer_obj);
                err_return = add_token(lexer_obj, DOUBLE_PIPE);
            }
            else {
                err_return = add_token(lexer_obj, PIPE);
            }
            break;

        case '&':
            if ('&' == lex_get_curr_char(lexer_obj)) {
                lex_advance_current(lexer_obj);
                err_return = add_token(lexer_obj, DOUBLE_AMPRSND);
            }
            else {
                err_return = add_token(lexer_obj, AMPRSND);
            }
            break;

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


Token *
tokenize(const char *input)
{
    struct Lexer_obj *lexer_obj = lex_init_obj(input);
    if (lexer_obj == NULL) {
        return NULL;
    }

    /* Main tokenizer loop */
    while (lex_current_at_end(lexer_obj) == false) {
        /* Move to the next lexeme */
        lexer_obj->start = lexer_obj->current;

        if (scan_token(lexer_obj) == -1) {
            destroy_lexer_obj(lexer_obj);
            return NULL;
        }
    }

    /* Add `NIL` as last token */
    if (add_token(lexer_obj, NIL) == -1) {
        destroy_lexer_obj(lexer_obj);
        return NULL;
    }

    /* Only need to return `tokens` array */
    Token *tokens = lexer_obj->tokens;
    free(lexer_obj);
    return tokens;
}
