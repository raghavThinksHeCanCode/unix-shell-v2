#include <stdio.h>

#include "executor.h"
#include "ast.h"
#include "job.h"


static int find_root_return_stat(Ast_node *root);
static void traverse_and_eval_ast(Ast_node *root);
static void execute_unit(Execution_unit *unit);


static int
find_root_return_stat(Ast_node *root)
{
    Node_type root_type = root->type;
    int left_status  = root->left->return_status;
    int right_status = root->right->return_status;

    if (root_type == AND) {
        if (left_status == 0 && right_status == 0) {
            return 0;
        }
        else if (left_status == 0 && right_status != 0) {
            return right_status;
        }
        else if (left_status != 0) {
            return left_status;
        }
    }

    else if (root_type == OR) {
        if (left_status != 0 && right_status != 0) {
            return right_status;
        }
        else if (left_status != 0 && right_status == 0) {
            return 0;
        }
        else if (left_status == 0) {
            return 0;
        }
    }
}


static void
traverse_and_eval_ast(Ast_node *root)
{
    if (root->type == JOB) {
        int return_status = launch_job(root->job);
        root->return_status = return_status;
        return;
    }

    traverse_and_eval_ast(root->left);

    if ((root->type == AND && root->left->return_status == 0)
     || (root->type == OR  && root->left->return_status != 0)) {
    
        traverse_and_eval_ast(root->right);
    }

    root->return_status = find_root_return_stat(root);
}


static void
execute_unit(Execution_unit *unit)
{
    //TODO: Check for foreground and background goes here
    traverse_and_eval_ast(unit->ast_root);
}


void
execute(Execution_unit *head)
{
    Execution_unit *ptr = head;
    while (ptr != NULL) {
        execute_unit(ptr);
        ptr = ptr->next;
    }
}