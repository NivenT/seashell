#ifndef COMPLETIONS_H_INCLUDED
#define COMPLETIONS_H_INCLUDED

#include <linenoise.h>

#include "defs.h"

#define COMPLETION_COMMON_FILE ".seashell_completions"
#define COMPLETION_MIN_LEN     2

extern void completion(const char* buf, linenoiseCompletions *lc);
extern void init_completions();

#endif // COMPLETIONS_H_INCLUDED
