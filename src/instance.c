#include "instance.h"
#include "ast.h"

#include <stddef.h>
#include <stdlib.h>


void
destroy_instance(Instance *instance)
{
    Instance *ptr = instance;
    while (ptr != NULL) {
        if (ptr->ast_root != NULL) {
            destroy_ast(ptr->ast_root);
        }
        Instance *temp = ptr;
        ptr = ptr->next;
        free(temp);
    }
}


Instance *
get_instance_obj(void)
{
    Instance *instance = malloc(sizeof(*instance));
    if (instance == NULL) {
        return NULL;
    }

    instance->ast_root = NULL;
    instance->context  = FOREGROUND; /* Default context for instance */
    instance->next     = NULL;
    return instance;
}