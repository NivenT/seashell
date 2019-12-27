#ifndef EXPRESSION_H_INCLUDED
#define EXPRESSION_H_INCLUDED

#include "pipeline.h"

typedef enum {ALL = 0, ANY = 1, LEAF = 2} expression_t;
typedef struct expression_node expression_node;
typedef struct expression expression;
typedef struct expressionlist exprlist;

// I was thinking of this as a tree before, but it's really a linked list the way I've done things
struct expression_node {
  expression_t type;
  // I don't like this asymettrey but it makes implementing things simpler
  // TODO: Rename these
  pipeline* lhs;
  expression_node* rhs;
};

struct expression {
  expression_node* head;
  bool fg;
  // I don't like this either
  size_t head_id; // the id of the job corresponding to the pipeline in head
};

// This feels so weird but is the best solution to implementing expressions
// I've thought of in terms of balancing correctness and complexity
struct expressionlist {
  vec exprs;
  bool fg;
};

extern void init_expressions();

extern bool build_expression(vec* tkns, expression* expr);
extern bool execute_expression(expression* expr);
extern void print_expression(expression* expr);
extern void free_expression(expression* expr);

extern bool el_has_fg();
extern expression* el_new_expr(vec* tkns);
extern void el_update_exprs(size_t id, int exit_status);

#endif // EXPRESSION_H_INCLUDED
