#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "defs.h"

extern bool starts_with(const char* str, const char* prefix);
extern bool ends_with(const char* str, const char* suffix);

// mallocs
extern char* join(const char** beg, const char** end, const char* delim);
extern char* concat(const char* str1, const char* str2);
extern char* concat_many(const char** strs); // this is basically join without the delim now that I think about it

// modifies argument
extern char* trim(char* str);

extern bool write_all(int fd, const void* buf, size_t count);

extern int nxtpwr2(int x);


#endif // UTILS_H_INCLUDED
