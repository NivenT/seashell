#ifndef EXPRESSION_H_INCLUDED
#define EXPRESSION_H_INCLUDED

#include "pipeline.h"

typedef enum {ALL, ANY, LEAF} expression_t;
typedef struct expression expression;

struct expression {
  expression_t type;
  pipeline* pipe;
  union {
    expression* chlds[2];
    struct {
      expression* lhs;
      expression* rhs;
    };
  };
};

extern bool build_expression(vec* tkns, expression* expr);
extern bool execute_expression(expression* expr, job* j);
extern void print_expression(expression* expr);
extern void free_expression(expression* expr);

#endif // EXPRESSION_H_INCLUDED
