#ifndef MYSTRING_H_INCLUDED
#define MYSTRING_H_INCLUDED

#define STRING_CAPACITY_HINT 8

// Should I use this more?
typedef struct {
  char* cstr; // always malloc'd and still null-terminated
  int len;
  int cap; // includes 0 at end
} string;

extern string string_new(char* str);
extern void string_grow(string* str, int new_capacity);
extern void string_push(string* str, char c);
extern void string_append(string* str, const char* s);
extern void string_appendn(string* str, const char* s, int n);
extern void string_replace(string* str, int pos, int len, const char* replacement);
extern int string_find(const string* str, int start, const char* s);
extern void free_string(string* str);

#endif // MYSTRING_H_INCLUDED
