#include <string.h>

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
