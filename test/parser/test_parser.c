#include <stdio.h>

#include "lexer.h"
#include "token.h"
#include "parser.h"


typedef struct Test_case
{
    int output;
    char input_string[256];
} Test_case;


Test_case tests[] = {
    {0, "ls"},
    {0, "ls -al"},
    {0, "ls -al && cd"},
    {0, "ls    ; cd"},
    {-1, "ls ;; cd"},
    {0, "ls -al | grep hello && cd"},
};


int
main(void)
{
    int tests_count = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < tests_count; i++) {
        int expected_output = tests[i].output;
        char *input_string = tests[i].input_string;

        Token *tokens = tokenize(input_string);
        int actual_output = parse_tokens(tokens);

        if (actual_output != expected_output) {
            printf("Failed: TESTCASE %d - %s\n", i + 1, input_string);
        }

        else {
            printf("Passed: Testcase %d - %s\n", i + 1, input_string);
        }
    }
}
