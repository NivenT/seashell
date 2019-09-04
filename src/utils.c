#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

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
    int new_len = len + strlen(*ptr);
    new_len += ptr == beg ? 0 : delim_len;
    ret = realloc(ret, new_len + 1);

    if (ptr != beg) {
      strcpy(ret + len, delim);
      strcpy(ret + len + delim_len, *ptr);
    } else {
      strcpy(ret, *ptr);
    }

    len = new_len;
  }
  return ret;
}

char* concat(const char* str1, const char* str2) {
  if (!str1) return strdup(str2);
  if (!str2) return strdup(str1);
  int len1 = strlen(str1), len2 = strlen(str2);

  char* ret = malloc(len1+len2+1);
  strcpy(ret, str1);
  strcpy(ret + len1, str2);
  return ret;
}

// TODO: Make more efficient
char* concat_many(const char** strs) {
  if (!strs) return NULL;
  int len = 0;
  for (const char** str = strs; *str; ++str) {
    len += strlen(*str);
  }

  char* ret = malloc(len+1);
  char* curr = ret;
  for (const char** str = strs; *str; ++str) {
    strcpy(curr, *str);
    curr += strlen(*str);
  }
  return ret;
}

const char* rsplit(const char* str, const char* sep, char** before) {
  if (!str || !sep) return before ? *before = NULL : NULL;
  int len = strlen(str);
  int len2 = strlen(sep);

  for (const char* cur = str + len - 1; cur >= str; --cur) {
    if (strncmp(cur, sep, len2) == 0) {
      if (before) *before = strndup(str, cur - str);
      return cur + len2;
    }
  }
  if (before) *before = NULL;
  return str;
}

char* first_word(const char* str) {
  if (!str) return NULL;
  const char* end = str;
  while (*end && !isspace(*end)) ++end;
  return strndup(str, end - str);
}

char* replace_all(const char* str, const char* old, const char* new) {
  if (!str || !old || !new) return NULL;
  string ret = string_new(NULL);

  int old_len = strlen(old);
  
  const char* beg = str;
  for (const char* it = str; *it; ++it) {
    if (strncmp(it, old, old_len) == 0) {
      string_appendn(&ret, beg, it-beg);
      string_append(&ret, new);
      it += old_len;
      beg = it--;
    }
  }
  if (*beg) string_append(&ret, beg);

  return ret.cstr;
}

bool write_all(int fd, const void* buf, size_t count) {
  for (size_t remaining = count; remaining > 0;) {
    int written = write(fd, buf, remaining);
    if (written == -1) return false;
    buf += written;
    remaining -= written;
  }
  return true;
}

char* trim(char* str) {
  if (!str) return NULL;
  while (isspace(*str) && *str) str++;
  char* end = str;
  while (*end) end++;
  while (end != str && (isspace(*(end-1)))) end--;
  *end = '\0';
  return str;
}

// "hi there" -> "there"
// "hi " -> ""
const char* last_word(const char* str) {
  if (!str) return NULL;
  const char* ret = str + strlen(str) - 1;
  while (ret > str && !isspace(*ret)) --ret;
  return isspace(*ret) ? ret + 1 : ret;
}

int nxtpwr2(int x) {
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x++;
  return x;
}

void closeall(int fds[], int nfds) {
  for (int j = 0; j < nfds; j++) {
    if (fds[j] >= 0) close(fds[j]);
  }
}
