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


static int
add_token(struct Lexer_obj *lexer_obj, Token_type type)
{
    if (lexer_obj->tok_count == MAX_TOK_COUNT) {
        fprintf(stderr, "Max token count exceeded\n");
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
        return add_word(lexer_obj);
    }

    return 0;
}


static int
handle_word(struct Lexer_obj *lexer_obj)
{
    char curr_ch = LEX_GET_CURR_CHAR(lexer_obj);

    /* Move current ahead, until any of the recognised lexeme is not found */
    while (curr_ch != ' ' && curr_ch != '\t' && curr_ch != '\0'
        && curr_ch != ';' && curr_ch != '&'  && curr_ch != '|'
        && curr_ch != '>' && curr_ch != '<') {

        lex_advance_current(lexer_obj);
        curr_ch = LEX_GET_CURR_CHAR(lexer_obj);

        if (CURR_LEXEME_SIZE(lexer_obj) > MAX_LEXEME_SIZE) {
            fprintf(stderr, "Maximum allowed lexeme size exceeded\n");
            return -1;
        }
    }

    return add_token(lexer_obj, NAME);
}


/* Identify lexeme starting from `start` to `current` in the string */
static int
scan_token(struct Lexer_obj *lexer_obj)
{
    /* Get the first character of current lexeme */
    const char c   = lex_advance_current(lexer_obj);
    int err_return = 0;

    switch (c) {
        case ' ': case '\t':
            /* Skip whitespaces */
            return 0;

        case ';':
            return add_token(lexer_obj, SEMICLN);

        case '\\':  /* Match `\` */
            return add_token(lexer_obj, BACKSLSH);

        case '|':
            if ('|' == LEX_GET_CURR_CHAR(lexer_obj)) {
                lex_advance_current(lexer_obj);
                return add_token(lexer_obj, DOUBLE_PIPE);
            }
            else {
                return add_token(lexer_obj, PIPE);
            }

        case '&':
            if ('&' == LEX_GET_CURR_CHAR(lexer_obj)) {
                lex_advance_current(lexer_obj);
                return add_token(lexer_obj, DOUBLE_AMPRSND);
            }
            else {
                return add_token(lexer_obj, AMPRSND);
            }

        /* ==== Word tokens ==== */
        default:
            return handle_word(lexer_obj);
    }
}


Token *
tokenize(const char *input)
{
    struct Lexer_obj *lexer_obj = get_lexer_obj(input);
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
