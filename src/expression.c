#include <stdio.h>
#include <stdlib.h>

#include "expression.h"
#include "parser.h"

bool build_expression(vec* tkns, expression* expr) {
  vec pre_op = vec_new(sizeof(token), 0, NULL);
  for (int i = 0; i < tkns->size; ++i) {
    const token* tkn = (const token*)vec_get(tkns, i);
    // maybe I should use switch instead...
    if (tkn->type == AND || tkn->type == OR) {
      expr->type = tkn->type == AND ? ALL : ANY;
      expr->pipe = NULL;

      expr->lhs = malloc(sizeof(expression));
      bool succ = build_expression(&pre_op, expr->lhs);
      free_vec(&pre_op);
      if (!succ) {
	free(expr->lhs);
	expr->lhs = NULL;
	return false;
      }
      
      vec tail = vec_tail(tkns, i+1);
      expr->rhs = malloc(sizeof(expression));
      if (!build_expression(&tail, expr->rhs)) {
	free(expr->rhs);
	expr->rhs = NULL;
	return false;
      }
      
      return true;
    } else {
      vec_push(&pre_op, vec_get(tkns, i));
    }
  }
  free_vec(&pre_op);

  expr->type = LEAF;
  expr->lhs = expr->rhs = NULL;
  expr->pipe = malloc(sizeof(pipeline));
  if (!build_pipeline(tkns, expr->pipe)) {
    free(expr->pipe);
    expr->pipe = NULL;
    return false;
  }
  return true;
}

bool execute_expression(expression* expr, job* j) {
  // Gotta figure out a good way to surface exit status information
}

void print_expression(expression* expr) {
  if (!expr) return;
  switch(expr->type) {
  case ALL:
    printf("(ALL ");
    print_expression(expr->lhs);
    printf(" ");
    print_expression(expr->rhs);
    printf(")");
    break;
  case ANY:
    printf("(ANY ");
    print_expression(expr->lhs);
    printf(" ");
    print_expression(expr->rhs);
    printf(")");
    break;
  case LEAF:
    printf("(LEAF ");
    print_pipeline(expr->pipe);
    printf(")");
    break;
  }
}
