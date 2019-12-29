#include <stdlib.h>
#include <string.h>

#include "mystring.h"

string string_new(char* str) {
  string ret = { .len = 0, .cap = 0, .cstr = NULL };

  if (str) {
    ret.len = strlen(str);
    ret.cap = ret.len << 2;
  }
  if (ret.cap > 0) {
    ret.cstr = malloc(ret.cap);
    strcpy(ret.cstr, str);
  }
  
  return ret;
}

void string_grow(string* str, int min) {
  while (min >= str->cap) {
    str->cap = str->cap == 0 ? STRING_CAPACITY_HINT : str->cap << 1;
    str->cstr = realloc(str->cstr, str->cap);
  }
}

void string_push(string* str, char c) {
  string_grow(str, str->len + 1);
  str->cstr[str->len++] = c;
  str->cstr[str->len] = '\0';
}

void string_append(string* str, const char* s) {
  if (!s) return;
  int len = strlen(s);
  string_grow(str, str->len + len); 
  while (*s) str->cstr[str->len++] = *(s++);
  str->cstr[str->len] = '\0';
}

void string_appendn(string* str, const char* s, int n) {
  if (!s || n <= 0) return;
  string_grow(str, str->len + n);
  strncpy(str->cstr + str->len, s, n);
  str->len += n;
}

void string_replace(string* str, int pos, int len, const char* s) {
  if (!s || str->len < pos + len) return;
  int slen = strlen(s);
  string_grow(str, str->len - len + slen);
  memmove(str->cstr + pos + slen, str->cstr + pos + len, str->len - pos - len);
  memcpy(str->cstr + pos, s, slen);
  str->len += slen - len;
  str->cstr[str->len] = '\0';
}

int string_find(const string* str, int start, const char* s) {
  if (!s || start > str->len) return -1;
  int slen = strlen(s);
  for (int pos = start; pos < str->len; ++pos) {
    if (strncmp(str->cstr + pos, s, slen) == 0) return pos;
  }
  return -1;
}

void free_string(string* str) {
  if (!str) return;
  free(str->cstr);
  str->len = str->cap = 0;
}
