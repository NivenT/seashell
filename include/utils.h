#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "defs.h"

/// I wonder how many of these are implemented as standard c functions that I just don't know about

extern bool starts_with(const char* str, const char* prefix);
extern bool starts_with_any(const char* str, const char* prefixes[]);
extern bool ends_with(const char* str, const char* suffix);

// mallocs
extern char* join(const char** beg, const char** end, const char* delim);
extern char* concat(const char* str1, const char* str2);
extern char* concat_many(const char** strs); // this is basically join without the delim now that I think about it
extern const char* rsplit(const char* str, const char* sep, char** before); // only before (if non-NULL) gets malloc'd
extern char* first_word(const char* str); // does not trim front
extern char* replace_all(const char* str, const char* old, const char* new);

// modifies argument
extern char* trim(char* str);

extern const char* last_word(const char* str);

// consistent casing, gotta love it
extern bool write_all(int fd, const void* buf, size_t count);
extern void closeall(int fds[], int nfds);

extern int nxtpwr2(int x);

#endif // UTILS_H_INCLUDED
