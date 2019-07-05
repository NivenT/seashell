#ifndef READLINE_H_INCLUDED
#define READLINE_H_INCLUDED

#include <linenoise.h>

#include "defs.h"

#define LINENOISE_HISTORY_FILE ".seashell_history"

extern void init_linenoise();
extern bool read_line(char* line);

extern char* history_file;

#endif // READLINE_H_INCLUDED
