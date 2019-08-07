#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "defs.h"

// EOI is a special "end of input" token used to make the code for build_pipeline slightly nicer
typedef enum {SYMBOL, STRING, PIPE, AMPERSAND, INFILE, OUTFILE, COMMENT, EOI} token_t;
typedef struct {
  token_t type;
  string str;
} token;

// Would it be better to return a token* instead for the type information?
extern vec parse_string(const char* line);
extern void print_tokens(const token* tkns, int num);

#endif // PARSER_H_INCLUDED
