#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "defs.h"

// Not sure how I feel about this style (e.g. vs what's used in commands.h)
typedef enum {SYMBOL, STRING, PIPE} token_t;
typedef struct {
  token_t type;
  string str;
} token;

// Would it be better to return a token* instead for the type information?
extern vec parse_string(const char* line);
extern void print_tokens(const token* tkns, int num);

#endif // PARSER_H_INCLUDED
