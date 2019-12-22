#ifndef EXPRESSION_H_INCLUDED
#define EXPRESSION_H_INCLUDED

#include "pipeline.h"

typedef enum {ALL, ANY, LEAF} expression_t;
typedef struct expression expression;

struct expression {
  expression_t type;
  // I don't like this asymettrey but it makes implementing things simpler
  pipeline* lhs;
  expression* rhs;
};

extern bool build_expression(vec* tkns, expression* expr);
extern bool execute_expression(expression* expr);
extern void print_expression(expression* expr);
extern void free_expression(expression* expr);

#endif // EXPRESSION_H_INCLUDED
