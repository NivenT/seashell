#include <stdio.h>

#include "parser.h"

// TODO: Actually manage memory in this program. There should be many more frees

void free_tkn(void* data) {
  if (!data) return;
  token* tkn = (token*)data;
  free_string(&tkn->str);
}

token tokenize(string s) {
  token ret;

  ret.type = SYMBOL;
  ret.str = s;
  
  return ret;
}

vec parse_string(const char* line) {
  vec tkns = new_vec(sizeof(token), 0, free_tkn);
  string curr = string_new(NULL);
  for (int i = 0; line[i]; i++) {
    switch(line[i]) {
    default: string_push(&curr, line[i]); break;
    }
  }

  token temp = tokenize(curr);
  vec_push(&tkns, &temp);
  return tkns;
}

void print_tokens(const token* tkns, int num) {
  if (!tkns || num == 0) return;
  for (int i = 0; i < num; i++) {
    switch(tkns[i].type) {
    case SYMBOL: case LITERAL: printf("%s ", tkns[i].str.cstr); break;
    case STRING: printf("\"%s\" ", tkns[i].str.cstr); break;
    case PIPE: printf("| "); break;
    }
  }
}
