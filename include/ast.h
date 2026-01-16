/*
    AST - Abstrct Syntax Tree is a representation of
    command execution flow in tree form. The ast takes
    conditional (`&&` and `||`).

    Each node of the AST is of type: AND, OR or PIPELINE.
    Each node also holds a pointer to its left and right
    child. Also, an extra field, `return_status` is
    present to dictate execution of next pipelines.

    For example, the tree for

        `pipeline1 || pipeline2 && pipeline3`
    
    will look like:

            (&&)
            /  \
           /    \
        (||)    (pipeline3)
        /  \
       /    \
(pipeline1)  (pipeline2)

    Execution will start from `pipeline1`. Based on its
    success, `pipeline2` will be launched. And similarly
    `pipeline3` will be executed.
*/


#ifndef AST_H_
#define AST_H_


#include "pipeline.h"


typedef enum Node_type
{
    AND, OR,  /* `&&` and `||` */
    PIPELINE,
} Node_type;


typedef struct Ast_node
{
    Node_type type;
    int       return_status;
    
    /* Left and right children */
    struct Ast_node *left;
    struct Ast_node *right;
    
    /* Only for node of type `Pipeline`.
       `NULL` for rest */
    Pipeline *pipeline;
} Ast_node;


Ast_node *create_ast_node(Node_type type, Pipeline *pipeline);
void destroy_ast(Ast_node *ast_root);


#endif // AST_H_
