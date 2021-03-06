#ifndef ALIAS_H_INCLUDED
#define ALIAS_H_INCLUDED

#include "defs.h"
#include "commands.h"

extern void init_aliases();
extern bool alias(const command cmd);
extern bool apply_aliases(char line[MAX_CMD_LEN]);

#endif // ALIAS_H_INCLUDED
