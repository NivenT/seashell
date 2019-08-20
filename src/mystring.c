#include <stdlib.h>
#include <string.h>

#include "mystring.h"
#include "utils.h"

string string_new(char* str) {
  string ret;
  
  ret.len = str ? strlen(str) : 0;
  ret.cap = str ? nxtpwr2(ret.len+1) : 0;
  ret.cstr = NULL;
  if (ret.cap > 0) {
    ret.cstr = malloc(ret.cap);
    strcpy(ret.cstr, str);
  }
  
  return ret;
}

void string_push(string* str, char c) {
  if (str->len + 1 >= str->cap) {
    str->cap = str->cap == 0 ? STRING_CAPACITY_HINT : str->cap << 1;
    str->cstr = realloc(str->cstr, str->cap);
  }
  str->cstr[str->len++] = c;
  str->cstr[str->len] = '\0';
}

void string_append(string* str, const char* s) {
  if (!s) return;
  int len = strlen(s);
  while (str->len + len >= str->cap) {
    str->cap = str->cap == 0 ? STRING_CAPACITY_HINT : str->cap << 1;
    str->cstr = realloc(str->cstr, str->cap);
  }
  while (*s) str->cstr[str->len++] = *(s++);
  str->cstr[str->len] = '\0';
}

void string_appendn(string* str, const char* s, int n) {
  if (!s || n <= 0) return;
  // I should really just add a function for chainging the string's capacity instead
  while(str->len + n >= str->cap) {
    str->cap = str->cap == 0 ? STRING_CAPACITY_HINT : str->cap << 1;
    str->cstr = realloc(str->cstr, str->cap);
  }
  strncpy(str->cstr + str->len, s, n);
  str->len += n;
}

void free_string(string* str) {
  if (str) {
    if (str->cstr) free(str->cstr);
    str->len = str->cap = 0;
  }
}
