#ifndef APT_REPOS_H_INCLUDED
#define APT_REPOS_H_INCLUDED

#include "defs.h"

// Calls "apt-cache search ." once and stores the results for later use
extern void init_apts();
extern const vec* apts_starting_with(char c);

// Calls "apt-cache search ." on the fly
// Uses like 2MiB less memory, but I think is too slow to justify
extern vec apts_starting_with_sp(const char* prefix);

#endif // APT_REPOS_H_INCLUDED
