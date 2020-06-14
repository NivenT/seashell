#ifndef NETWORKING_H_INCLUDED
#define NETWORKING_H_INCLUDED

#include "defs.h"

extern void init_networking();
extern string http_simple_get(const char* url);
extern vec* get_repos(char* user);

#endif // NETWORKING_H_INCLUDED
