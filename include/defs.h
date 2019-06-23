#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <stdbool.h>

#define PROMPT "seashell> "

#define MAX_CMD_LEN 256
#define MAX_ERR_LEN 256

#define MAX_CMD_NAME_LEN 32
#define MAX_NUM_ARGS 32

extern char error_msg[MAX_ERR_LEN];

#endif // DEFS_H_INCLUDED
