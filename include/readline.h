#ifndef READLINE_H_INCLUDED
#define READLINE_H_INCLUDED

#include <linenoise.h>

#include "defs.h"

#define LINENOISE_HISTORY_FILE    ".seashell_history"
#define LINENOISE_HISTORY_MAX_LEN 1024

extern char* history_file;

extern void init_linenoise();
extern bool read_line(char* line);

#endif // READLINE_H_INCLUDED
