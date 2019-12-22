#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expression.h"
#include "parser.h"
#include "job.h"

bool build_expression(vec* tkns, expression* expr) {
  vec pre_op = vec_new(sizeof(token), 0, NULL);
  for (int i = 0; i < tkns->size; ++i) {
    const token* tkn = (const token*)vec_get(tkns, i);
    // maybe I should use switch instead...
    if (tkn->type == AND || tkn->type == OR) {
      expr->type = tkn->type == AND ? ALL : ANY;
      
      expr->lhs = malloc(sizeof(pipeline));
      bool succ = build_pipeline(&pre_op, expr->lhs);
      free_vec(&pre_op);
      if (!succ) return false;
      
      vec tail = vec_tail(tkns, i+1);
      expr->rhs = malloc(sizeof(expression));
      if (!build_expression(&tail, expr->rhs)) return false;
      
      return true;
    } else {
      vec_push(&pre_op, vec_get(tkns, i));
    }
  }
  free_vec(&pre_op);

  expr->type = LEAF;
  expr->rhs = NULL;
  expr->lhs = malloc(sizeof(pipeline));
  return build_pipeline(tkns, expr->lhs);
}

static job* run_pipe(pipeline* pipe) {
  job* j = jl_new_job(pipe->fg);
  if (execute_pipeline(pipe, j)) finish_job_prep(j);
  return j;
}

bool execute_expression(expression* expr) {
  if (!expr) {
    strcpy(error_msg, "Tried executing a NULL expression");
    return false;
  }
  job* j = run_pipe(expr->lhs);
  
  /*
    CHECK_ERROR(error, build_pipeline(&tkns, &pipe));
    free_vec(&tkns);
    job* j = jl_new_job(pipe.fg);
    CHECK_ERROR(error, execute_pipeline(&pipe, j));
    CHECK_ERROR(error, finish_job_prep(j));
    free_pipeline(&pipe);
  */
}

void free_expression(expression* expr) {
  if (!expr) return;
  free_pipeline(expr->lhs);
  free(expr->lhs);
  free_expression(expr->rhs);
  free(expr->rhs);
}

// This function is trash
void print_expression(expression* expr) {
  if (!expr) return;
  switch(expr->type) {
  case ALL:
    printf("(ALL ");
    print_pipeline(expr->lhs);
    printf(" ");
    print_expression(expr->rhs);
    printf(")");
    break;
  case ANY:
    printf("(ANY ");
    print_pipeline(expr->lhs);
    printf(" ");
    print_expression(expr->rhs);
    printf(")");
    break;
  case LEAF:
    printf("(LEAF ");
    print_pipeline(expr->lhs);
    printf(")");
    break;
  }
}
