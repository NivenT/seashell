#include <stdio.h>
#include <ctype.h>

#include "parser.h"

// TODO: Actually manage memory in this program. There should be many more frees

void free_tkn(void* data) {
  if (!data) return;
  token* tkn = (token*)data;
  free_string(&tkn->str);
}

void add_tkn(vec* tkns, string* s) {
  if (!s || s->len == 0) return;
  token temp;

  temp.type = SYMBOL;
  temp.str = *s;
  
  vec_push(tkns, &temp);
  *s = string_new(NULL);
}

vec parse_string(const char* line) {
  vec tkns = new_vec(sizeof(token), 0, free_tkn);
  string curr = string_new(NULL);
  for (int i = 0; line[i]; i++) {
    if (curr.len > 0 && curr.cstr[0] == '\"') {
      string_push(&curr, line[i]);
      if (line[i] == '\"') {
	add_tkn(&tkns, &curr);
      }
    } else {
      if (isspace(line[i])) {
	add_tkn(&tkns, &curr);
      } else {
	string_push(&curr, line[i]);
      }
    }
  }
  add_tkn(&tkns, &curr);
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