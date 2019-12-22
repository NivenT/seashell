#ifndef EXPRESSION_H_INCLUDED
#define EXPRESSION_H_INCLUDED

#include "pipeline.h"

typedef enum {ALL, ANY, LEAF} expression_t;
typedef struct expression_node expression_node;
typedef struct expression expression;

// I was thinking of this as a tree before, but it's really a linked list the way I've done things
struct expression_node {
  expression_t type;
  // I don't like this asymettrey but it makes implementing things simpler
  // TODO: Rename these
  pipeline* lhs;
  expression_node* rhs;
};

struct expression {
  expression_node head;
  bool fg;
};

extern bool expr_in_fg;

extern bool build_expression(vec* tkns, expression* expr);
extern bool execute_expression(expression* expr);
extern void print_expression(expression* expr);
extern void free_expression(expression* expr);

#endif // EXPRESSION_H_INCLUDED
