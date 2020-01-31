#ifndef READLINE_H_INCLUDED
#define READLINE_H_INCLUDED

#include <linenoise.h>

#include "completions.h"
#include "defs.h"

#define LINENOISE_HISTORY_FILE    ".seashell_history"
#define LINENOISE_HISTORY_MAX_LEN 1024

#define HINTS_COMMON_FILE  ".seashell_hints"
#define HINTS_MIN_LEN      2
#define HIST_HINT_MIN_FREQ 3
#define HIST_HINT_MIN_LEN  (HINTS_MIN_LEN + 1)
#define HIST_HINT_NUM_RECS 10

extern char* history_file;

extern void init_linenoise();
extern bool read_line(char* line);

#endif // READLINE_H_INCLUDED
