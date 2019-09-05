#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#if defined(__linux__)
#define OSLINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define OSMAC
#else
#define OSUNKNOWN
#endif

#include <stdbool.h>

#include "mystring.h"
#include "myvec.h"
#include "mymap.h"

//#define PROMPT "\2\e[1;36m\2seashell:\2\e[0m\2%s\2\e[1;36m\2>\2\e[0m\2 "
#define PROMPT "seashell:%s> "

#define NOT_IMPLEMENTED(name) printf("%s has not yet been implemented\n", name)

#define MAX_NUM_LEN 32
#define MAX_PTH_LEN 256
#define MAX_PROMPT_LEN (MAX_PTH_LEN + 39)
#define MAX_BM_LINE_LEN (2*MAX_PTH_LEN+1)
#define MAX_CMD_LEN 4096
#define MAX_ERR_LEN 256
#define MAX_CMD_NAME_LEN 64
#define MAX_NUM_ARGS 128

extern char error_msg[MAX_ERR_LEN];
extern char* home_dir;

#endif // DEFS_H_INCLUDED
