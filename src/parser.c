#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <glob.h>
#include <stdlib.h>

#include "parser.h"

static void free_tkn(void* data) {
  // The vector returned by parse_string does not own the undrlying char*'s
}

static void glob_tkns(vec* tkns, const char* patt) {
  glob_t g;
  glob(patt, GLOB_NOCHECK | GLOB_BRACE | GLOB_NOMAGIC | GLOB_TILDE, NULL, &g);
  for (int i = 0; i < g.gl_pathc; i++) {
    token t = {SYMBOL, string_new(g.gl_pathv[i])};
    vec_push(tkns, &t);
  }
  globfree(&g);
}

static void add_tkn(vec* tkns, string* s) {
  if (!s || s->len == 0) return;
  token temp;
  
  if (s->len == 1 && s->cstr[0] == '|') {
    temp.type = PIPE;
    free_string(s);
  } else if (s->len == 1 && s->cstr[0] == '&') {
    temp.type = AMPERSAND;
    free_string(s);
  } else if (s->len == 1 && s->cstr[0] == '>') {
    temp.type = OUTFILE;
    free_string(s);
  } else if (s->len == 1 && s->cstr[0] == '<') {
    temp.type = INFILE;
    free_string(s);
  } else if (s->len > 0 && s->cstr[0] == '#') {
    temp.type = COMMENT;
    temp.str = *s;
  } else if (s->len >= 2 && ((s->cstr[0] == '\"' && s->cstr[s->len-1] == '\"') ||
			     (s->cstr[0] == '\'' && s->cstr[s->len-1] == '\''))) {
    // (manually) get rid of quotation marks
    memmove(s->cstr, s->cstr + 1, s->len - 1);
    s->cstr[s->len-2] = '\0';
    s->len -= 2;

    temp.type = STRING;
    temp.str = *s;
  } else if (s->len == 2 && s->cstr[0] == '\\') {
    memmove(s->cstr, s->cstr + 1, s->len--);

    temp.type = SYMBOL;
    temp.str = *s;
  } else if (s->len == 2 && s->cstr[0] == '&' && s->cstr[1] == '&') {
    temp.type = AND;
    free_string(s);
  } else if (s->len == 2 && s->cstr[0] == '|' && s->cstr[1] == '|') {
    temp.type = OR;
    free_string(s);
  } else {
    glob_tkns(tkns, s->cstr);
    free_string(s);
    *s = string_new(NULL);
    return;
  }
  
  vec_push(tkns, &temp);
  *s = string_new(NULL);
}

vec parse_string(const char* line) {
  vec tkns = vec_new(sizeof(token), 0, free_tkn);
  string curr = string_new(NULL);
  for (int i = 0; line[i]; i++) {
    if (curr.len > 0 && curr.cstr[0] == '#') {
      string_push(&curr, line[i]);
    } else if (curr.len > 0 && curr.cstr[curr.len-1] == '\\') {
      curr.cstr[curr.len-1] = line[i];
    } else if (curr.len > 0 && curr.cstr[0] == '\"') {
      string_push(&curr, line[i]);
      if (line[i] == '\"') {
	add_tkn(&tkns, &curr);
      }
    } else if (curr.len > 0 && curr.cstr[0] == '\'') {
      string_push(&curr, line[i]);
      if (line[i] == '\'') {
	add_tkn(&tkns, &curr);
      }
    } else {
      if (isspace(line[i])) {
	add_tkn(&tkns, &curr);
      } else if (line[i] == '|' && line[i+1] == '|' && curr.len == 0) {
	string_appendn(&curr, &line[i++], 2);
	add_tkn(&tkns, &curr);
      } else if (line[i] == '|' && curr.len == 0) {
	string_push(&curr, line[i]);
	add_tkn(&tkns, &curr);
      } else if (line[i] == '&' && line[i+1] == '&' && curr.len == 0) {
	string_appendn(&curr, &line[i++], 2);
	add_tkn(&tkns, &curr);
      } else if (line[i] == '&' && curr.len == 0) {
	string_push(&curr, line[i]);
	add_tkn(&tkns, &curr);
      } else if (line[i] == '<' && curr.len == 0) {
	string_push(&curr, line[i]);
	add_tkn(&tkns, &curr);
      } else if (line[i] == '>' && curr.len == 0) {
	string_push(&curr, line[i]);
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
    case SYMBOL: printf("SYMBOL(%s) ", tkns[i].str.cstr); break;
    case STRING: printf("STRING(%s) ", tkns[i].str.cstr); break;
    case PIPE: printf("PIPE "); break;
    case AMPERSAND: printf("AMPERSAND "); break;
    case INFILE: printf("INFILE "); break;
    case OUTFILE: printf("OUTFILE "); break;
    case COMMENT: printf("COMMENT(%s) ", tkns[i].str.cstr); break;
    case AND: printf("AND "); break;
    case OR: printf("OR "); break;
    case EOI: printf("EOI "); break;
    }
  }
}

void vprint_tokens(vec* tkns) {
  print_tokens(tkns->data, tkns->size);
}
