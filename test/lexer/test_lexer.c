#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "token.h"


typedef struct Test_case
{
    const char *input_string;
    Token output_tokens[MAX_TOK_COUNT];
} Test_case;


Test_case tests[] = {
    {"|", {{PIPE, NULL}, {NIL, NULL}}},
    {"||", {{LOGIC_OR, NULL}, {NIL, NULL}}},
    {"|||", {{LOGIC_OR, NULL}, {PIPE, NULL}, {NIL, NULL}}},
    {"||||", {{LOGIC_OR, NULL}, {LOGIC_OR, NULL}, {NIL, NULL}}},
    {"&", {{BG_OPERATOR, NULL}, {NIL, NULL}}},
    {"&&", {{LOGIC_AND, NULL}, {NIL, NULL}}},
    {"&&&", {{LOGIC_AND, NULL}, {BG_OPERATOR, NULL}, {NIL, NULL}}},
    {"&&&&", {{LOGIC_AND, NULL}, {LOGIC_AND, NULL}, {NIL, NULL}}},
    {";", {{SEMICOLON, NULL}, {NIL, NULL}}},
    {";;", {{SEMICOLON, NULL}, {SEMICOLON, NULL}, {NIL, NULL}}},
    {";;;", {{SEMICOLON, NULL}, {SEMICOLON, NULL}, {SEMICOLON, NULL}, {NIL, NULL}}},
    {">", {{RIGHT_REDIR, NULL}, {NIL, NULL}}},
    {">>", {{DOUBLE_RIGHT_REDIR, NULL}, {NIL, NULL}}},
    {">>>", {{DOUBLE_RIGHT_REDIR, NULL}, {RIGHT_REDIR, NULL}, {NIL, NULL}}},
    {">>>>", {{DOUBLE_RIGHT_REDIR, NULL}, {DOUBLE_RIGHT_REDIR, NULL}, {NIL, NULL}}},
    {"<", {{LEFT_REDIR, NULL}, {NIL, NULL}}},
    {"<<", {{LEFT_REDIR, NULL}, {LEFT_REDIR, NULL}, {NIL, NULL}}},
    {"<<<", {{LEFT_REDIR, NULL}, {LEFT_REDIR, NULL}, {LEFT_REDIR, NULL}, {NIL, NULL}}},
    {"<<<<", {{LEFT_REDIR, NULL}, {LEFT_REDIR, NULL}, {LEFT_REDIR, NULL}, {LEFT_REDIR, NULL}, {NIL, NULL}}},
    {"(", {{LEFT_PAREN, NULL}, {NIL, NULL}}},
    {"((", {{LEFT_PAREN, NULL}, {LEFT_PAREN, NULL}, {NIL, NULL}}},
    {"ls", {{CMD, "ls"}, {NIL, NULL}}},
    {"ls -al", {{CMD, "ls"}, {ARG, "-al"}, {NIL, NULL}}},
    {"ls -al && cd /", {{CMD, "ls"}, {ARG, "-al"}, {LOGIC_AND, NULL}, {CMD, "cd"}, {ARG, "/"}, {NIL, NULL}}},
    {"ls -al | grep file > test", {{CMD, "ls"}, {ARG, "-al"}, {PIPE, NULL}, {CMD, "grep"}, {ARG, "file"}, {RIGHT_REDIR, NULL}, {CMD, "file"}}},
    {"(ls -al | grep file > test) && ((whoami || whatis) && sudo)", {{LEFT_PAREN, NULL}, {CMD, "ls"}, {ARG, "-al"}, {PIPE, NULL}, {CMD, "grep"}, {ARG, "file"}, {RIGHT_REDIR, NULL}, {CMD, "test"}, {RIGHT_PAREN, NULL}, {LOGIC_AND, NULL}, {LEFT_PAREN, NULL}, {LEFT_PAREN, NULL}, {CMD, "whoami"}, {LOGIC_OR, NULL}, {CMD, "whatis"}, {RIGHT_PAREN, NULL}, {LOGIC_AND, NULL}, {CMD, "sudo"}, {RIGHT_PAREN, NULL}, {NIL, NULL}}}
};


bool
check_lexer_output(Token *expected_output, Token *actual_output)
{
    for (size_t i = 0; actual_output[i].type != NIL; i++) {
        if (actual_output[i].type != expected_output[i].type) {
            return false;
        }

        if (actual_output[i].lexeme == NULL && expected_output[i].lexeme == NULL) {
            return true;
        }

        if (!strcmp(actual_output[i].lexeme, expected_output[i].lexeme)) {
            return true;
        }

        return false;
    }
}


int
main(void)
{
    size_t tests_count = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < tests_count; i++) {
        const char *input_string = tests[i].input_string;
        Token *expected_output = tests[i].output_tokens;
        Token *actual_output = tokenize(input_string);

        if (check_lexer_output(expected_output, actual_output) == true) {
            printf("\033[32mPass: Testcase %zu: %s\n", i + 1, input_string);
        }
        else {
            printf("\033[31mFail: Testcase %zu: %s\n", i + 1, input_string);
        }

        free(actual_output);
    }
}
