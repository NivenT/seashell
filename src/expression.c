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
  free_expression(*(expression**)addr);
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

static bool execute_expression_node(expression* root, expression_node* node, bool fg) {
  if (!node) {
    strcpy(error_msg, "Tried executing a NULL expression");
    return false;
  }

  job* j = jl_new_job(fg);
  root->head_id = j->id;

  printf("Executing pipeline \"");
  print_pipeline(node->lhs);
  printf("\"\n");
  
  if (!execute_pipeline(root, node->lhs, j)) return false;
  if (!finish_job_prep(j)) return false;
  printf("Began execution of expression with head_id %ld\n", root->head_id);
  el.fg = fg && jl_has_job(root->head_id) && !job_is_terminated(j);
  printf("el.fg = %d\n", el.fg);
  return true;
}

bool execute_expression(expression* expr) {
  return execute_expression_node(expr, expr->head, expr->fg);
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
  expr->head = expr->head->rhs;
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

void el_update_exprs(size_t id, int stat) {
  printf("Trying to update expressions\n");
  
  job* j = jl_get_job_by_id(id);
  printf("Found job %p\n", j);
  if (!j) return;

  printf("Here %ld\n", id);
  for (int i = 0; i < el.exprs.size; ++i) {
    expression* expr = (expression*)vec_get(&el.exprs, i);
    expression_node* node = expr->head;
    
    if (j->id == expr->head_id) {
      if ((stat == 0 && node->type == ALL) || (stat != 0 && node->type == ANY)) {
	advance_expression(expr);
	execute_expression(expr);
      } else {
	printf("Finished expression: %d -> ", el.fg);
	el.fg = el.fg && !expr->fg;
	printf("%d\n", el.fg);

	*expr = *(expression*)vec_back(&el.exprs);
	vec_pop(&el.exprs);
      }
      break;
    }
  }
  printf("Done\n");
}
