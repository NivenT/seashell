#include <stdio.h>

#include "parser.h"

// TODO: Actually manage memory in this program. There should be many more frees

token* parse_string(const char* line, int* num_tkns) {
  *num_tkns = 0;
  
  token* tkns = NULL;
  string curr = string_new(NULL);
  for (int i = 0; line[i]; i++) {
    switch(line[i]) {
    default: string_push(&curr, line[i]); break;
    }
  }
  return NULL;
}

void print_tokens(const token* tkns, int num) {
  if (!tkns || num == 0) return;
  for (int i = 0; i < num; i++) {
    switch(tkns[i].type) {
    case SYMBOL: case LITERAL: printf("%s ", tkns[i].str); break;
    case STRING: printf("\"%s\" ", tkns[i].str); break;
    case PIPE: printf("| "); break;
    }
  }
}
