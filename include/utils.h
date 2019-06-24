#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "defs.h"

extern bool starts_with(const char* str, const char* prefix);
extern bool ends_with(const char* str, const char* suffix);
// mallocs
extern char* join(const char** beg, const char** end, const char* delim);

#endif // UTILS_H_INCLUDED
