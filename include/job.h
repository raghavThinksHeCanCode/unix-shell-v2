/*

*/

#ifndef JOB_H_
#define JOB_H_


#include "ast.h"
#include <stdbool.h>


typedef struct Job
{
    bool        is_foreground;
    Ast_node   *ast_root;
    struct Job *next;
} Job;


Job *get_job_node(void);
void destroy_job_list(Job *head);


#endif // JOB_H_