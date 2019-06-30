#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <stdbool.h>

#include "mystring.h"
#include "myvec.h"

#define PROMPT "seashell:%s> "

#define NOT_IMPLEMENTED(name) printf("%s has not yet been implemented\n", name)

#define MAX_PTH_LEN 256
#define MAX_PROMPT_LEN (MAX_PTH_LEN + 11)
#define MAX_BM_LINE_LEN (2*MAX_PTH_LEN+1)
#define MAX_CMD_LEN 1024
#define MAX_ERR_LEN 256
#define MAX_CMD_NAME_LEN 64
#define MAX_NUM_ARGS 32

extern char error_msg[MAX_ERR_LEN];
extern char* home_dir;

#endif // DEFS_H_INCLUDED
