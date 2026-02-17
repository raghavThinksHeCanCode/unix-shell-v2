#include "exec_helper.h"

#include <stdbool.h>
#include <assert.h>


bool
can_execute_right_pipeline(Ast_node *node)
{
    assert(node->type == AND || node->type == OR);

    /* failed and (anything) = failed */
    if (node->type == AND && node->left->return_status != 0) {
        node->return_status = node->left->return_status;
        return false;
    }

    /* success or (anything) = success */
    if (node->type == OR  && node->left->return_status == 0) {
        node->return_status = node->left->return_status;
        return false;
    }

    return true;
}


void
update_node_status(Ast_node *node)
{
    assert(node->type == AND || node->type == OR);

    if (node->type == AND) {
        if (node->left->return_status != 0) {
            node->return_status = node->left->return_status;
        }
        else if (node->right->return_status != 0) {
            node->return_status = node->right->return_status;
        }
        else {
            node->return_status = 0;
        }
    }

    else if (node->type == OR) {
        if (node->left->return_status == 0) {
            node->return_status = 0;
        }
        else if (node->right->return_status == 0) {
            node->return_status = 0;
        }
        else {
            node->return_status = node->right->return_status;
        }
    }

}