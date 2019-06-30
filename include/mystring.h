#ifndef MYSTRING_H_INCLUDED
#define MYSTRING_H_INCLUDED

#define STRING_CAPACITY_HINT 8

typedef struct {
  char* cstr; // always malloc'd and still null-terminated
  int len;
  int cap; // includes 0 at end
} string;

extern string string_new(char* str);
extern void string_push(string* str, char c);
extern void free_string(string* str);

#endif // MYSTRING_H_INCLUDED
