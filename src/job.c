#include "job.h"
#include "ast.h"

#include <stdbool.h>
#include <stdlib.h>


void
destroy_job_list(Job *head)
{
    while (head != NULL) {
        if (head->ast_root != NULL) {
            destroy_ast(head->ast_root);
        }
        Job *temp = head;
        head = head->next;
        free(temp);
    }
}


Job *
get_job_node(void)
{
    Job *node = malloc(sizeof(*node));
    if (node == NULL) {
        return NULL;
    }

    node->ast_root       = NULL;
    node->is_foreground  = true; /* Default context */
    node->next           = NULL;
    return node;
}