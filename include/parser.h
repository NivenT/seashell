#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "defs.h"

// Not sure how I feel about this style (e.g. vs what's used in commands.h)
typedef enum {SYMBOL, STRING, PIPE, LITERAL} token_t;
typedef struct {
  token_t type;
  char* str;
} token;

extern token* parse_string(const char* line, int* num_tkns);
extern void print_tokens(const token* tkns, int num);

#endif // PARSER_H_INCLUDED
