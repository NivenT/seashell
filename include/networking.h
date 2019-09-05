#ifndef NETWORKING_H_INCLUDED
#define NETWORKING_H_INCLUDED

#include "defs.h"

/*
// returns socket
int http_simple_get(char* host, char* path, unsigned short port);
*/

void init_networking();
string http_simple_get(const char* url);

#endif // NETWORKING_H_INCLUDED
