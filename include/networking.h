#ifndef NETWORKING_H_INCLUDED
#define NETWORKING_H_INCLUDED

#include "defs.h"

void init_networking();
string http_simple_get(const char* url);
vec* get_repos(char* user);

#endif // NETWORKING_H_INCLUDED
