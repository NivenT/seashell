#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "expression.h"
#include "parser.h"
#include "job.h"
#include "utils.h"

exprlist el;

static void free_expr(void* addr) {
  free_expression((expression*)addr);
}

static void cleanup() {
  el.fg = false;
  free_vec(&el.exprs);
}

void init_expressions() {
  el.fg = false;
  el.exprs = vec_new(sizeof(expression), 0, free_expr);

  atexit(cleanup);
}

static bool build_expression_node(vec* tkns, expression_node* node) {
  node->lhs = NULL;
  node->rhs = NULL;

  vec pre_op = vec_new(sizeof(token), 0, NULL);
  for (int i = 0; i < tkns->size; ++i) {
    const token* tkn = (const token*)vec_get(tkns, i);
    switch(tkn->type) {
    case AND: case OR: {
      node->type = tkn->type == AND ? ALL : ANY;
      
      node->lhs = malloc(sizeof(pipeline));
      bool succ = build_pipeline(&pre_op, node->lhs);
      free_vec(&pre_op);
      if (!succ) return false;
      
      vec tail = vec_tail(tkns, i+1);
      node->rhs = malloc(sizeof(expression));
      if (!build_expression_node(&tail, node->rhs)) return false;
      
      return true;
    } break;
    case AMPERSAND:
      strcpy(error_msg, "Single Ampersands can only appear at the end of your input");
      return false;
      break;
    default: vec_push(&pre_op, vec_get(tkns, i)); break;
    }
  }
  free_vec(&pre_op);

  node->type = LEAF;
  node->rhs = NULL;
  node->lhs = malloc(sizeof(pipeline));
  return build_pipeline(tkns, node->lhs);
}

bool build_expression(vec* tkns, expression* expr) {
  expr->fg = true;
  if (tkns->size == 0) {
    strcpy(error_msg, "Cannot build an empty expression");
    return false;
  } else if (((token*)vec_back(tkns))->type == AMPERSAND) {
    expr->fg = false;
    vec_pop(tkns);
  }
  expr->head = malloc(sizeof(*expr->head));
  return build_expression_node(tkns, expr->head);
}

bool execute_expression(expression* expr) {
  if (!expr || !expr->head) {
    strcpy(error_msg, "Tried executing a NULL expression");
    return false;
  }
  expression_node* node = expr->head;

  job* j = jl_new_job(expr->fg);
  expr->head_id = j->id;

  if (!execute_pipeline(node->lhs, j)) return false;
  if (!finish_job_prep(j)) return false;
  el.fg = expr->fg && jl_has_job(expr->head_id) && !job_is_terminated(j);
  return true;
}

static void free_expression_node(expression_node* node) {
  if (!node) return;
  free_pipeline(node->lhs);
  free(node->lhs);
  free_expression_node(node->rhs);
  free(node->rhs);
}

void free_expression(expression* expr) {
  if (!expr) return;
  free_expression_node(expr->head);
  free(expr->head);
}

static void print_expression_node(expression_node* node) {
  if (!node) return;
  switch(node->type) {
  case ALL: case ANY:
    printf("(%s ", node->type == ALL ? "ALL" : "ANY");
    print_pipeline(node->lhs);
    printf(" ");
    print_expression_node(node->rhs);
    printf(")");
    break;
  case LEAF:
    printf("(LEAF ");
    print_pipeline(node->lhs);
    printf(")");
    break;
  }
}

void print_expression(expression* expr) {
  if (!expr) return;
  printf("(%s ", expr->fg ? "FG" : "BG");
  print_expression_node(expr->head);
  printf(")");
}

static void advance_expression(expression* expr) {
  if (!expr || !expr->head) return;
  free_pipeline(expr->head->lhs);

  void* temp = expr->head;
  expr->head = expr->head->rhs;
  free(temp);
}

bool el_has_fg() {
  return el.fg;
}

expression* el_new_expr(vec* tkns) {
  expression expr = { .head_id = 0 };
  if (!build_expression(tkns, &expr)) return NULL;
  vec_push(&el.exprs, &expr);
  return (expression*)vec_back(&el.exprs);
}

extern void regain_terminal_control(const pid_t seashell_pid);
void el_update_exprs(size_t id, int stat) {
  job* j = jl_get_job_by_id(id);
  if (!j) return;

  for (void* it = vec_first(&el.exprs); it; it = vec_next(&el.exprs, it)) {
    expression* expr = (expression*)it;
    expression_node* node = expr->head;
    
    if (j->id == expr->head_id) {
      if ((stat == 0 && node->type == ALL) || (stat != 0 && node->type == ANY)) {
	advance_expression(expr);
	if (expr->fg) regain_terminal_control(getpid());
	execute_expression(expr);
      } else {
	el.fg = el.fg && !expr->fg;

        swap(it, vec_back(&el.exprs), sizeof(expression));
	vec_pop(&el.exprs);
      }
      break;
    }
  }
}
