#ifndef EXEC_HELPER_H_
#define EXEC_HELPER_H_


#include "ast.h"

#include <stdbool.h>


/* For node of type `AND` or `OR`, determine if right child
   can be executed. If not, update the return status of the node */
bool can_execute_right_pipeline(Ast_node *node);

/* For node of type `AND` and `OR`, update its return
   status by determining the return status of both its
   children. Both children must have a valid return status. */
void update_node_status(Ast_node *node);


#endif // EXEC_HELPER_H_