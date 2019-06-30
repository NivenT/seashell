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

void free_string(string* str) {
  if (str) {
    if (str->cstr) free(str->cstr);
    str->len = str->cap = 0;
  }
}
