#include <string.h>
#include <stdlib.h>

#include "utils.h"

bool starts_with(const char* str, const char* prefix) {
  int len = strlen(prefix);
  return strncmp(str, prefix, len) == 0;
}

bool ends_with(const char* str, const char* suffix) {
  int len = strlen(str);
  int len2 = strlen(suffix);
  return len >= len2 && strncmp(str + (len - len2), suffix, len2) == 0;
}

char* join(const char** beg, const char** end, const char* delim) {
  char* ret = NULL;
  int len = 0;

  int delim_len = strlen(delim);
  for (const char** ptr = beg; ptr != end; ++ptr) {
    int new_len = len + delim_len + strlen(*ptr);
    ret = realloc(ret, new_len + 1);

    if (ptr != beg) strcpy(ret + len, delim);
    strcpy(ret + len + delim_len, *ptr);

    len = new_len;
  }
  return ret;
}