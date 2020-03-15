#ifndef HINTS_H_INCLUDED
#define HINTS_H_INCLUDED

#include <linenoise.h>

#include "defs.h"

#define HINTS_COMMON_FILE  ".seashell_hints"
#define HINTS_MIN_LEN      2
#define HIST_HINT_MIN_FREQ 3
#define HIST_HINT_MIN_LEN  (HINTS_MIN_LEN + 1)
#define HIST_HINT_NUM_RECS 10

extern char* hints(const char* buf, int* color, int* bold);
extern void init_hints();

#endif // HINTS_H_INCLUDED
