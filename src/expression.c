#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "expression.h"
#include "parser.h"
#include "job.h"

bool expr_in_fg = false;

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
  return build_expression_node(tkns, &expr->head);
}

static bool execute_expression_node(expression* root, expression_node* node, bool fg) {
  if (!node) {
    strcpy(error_msg, "Tried executing a NULL expression");
    return false;
  }

  job* j = jl_new_job(fg);
  if (!execute_pipeline(root, node->lhs, j)) return false;
  if (!finish_job_prep(j)) return false;
  if (node->type == LEAF) return true;
  expr_in_fg = fg;

  pid_t root_pid = getpid();
  pid_t pid = fork();
  if (pid == 0) {
    int status;
    if (waitpid(job_get_last_pid(j), &status, 0) == -1) {
      sprintf(error_msg, "waitpid errored: %s", strerror(errno));
      return false;
    }
    
    if (WIFEXITED(status)) {
      int stat = WEXITSTATUS(status);
      if ((stat == 0 && node->type == ALL) || (stat != 0 && node->type == ANY)) {
	execute_expression_node(root, node->rhs, fg);
      }
    } else if (WIFSIGNALED(status)) {
      if (node->type == ANY) execute_expression_node(root, node->rhs, fg);
    } else {
      kill(root_pid, SIGUSR1);
      strcpy(error_msg, "waitpid returned but child had not exited");
      return false;
    }
    if (fg) kill(root_pid, SIGUSR1);
    exit(0);
  }
  return true;
}

bool execute_expression(expression* expr) {
  return execute_expression_node(expr, &expr->head, expr->fg);
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
  free_expression_node(&expr->head);
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

// This function is trash
void print_expression(expression* expr) {
  if (!expr) return;
  printf("(%s ", expr->fg ? "FG" : "BG");
  print_expression_node(&expr->head);
  printf(")");
}
