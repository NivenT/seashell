#ifndef RCFILE_H_INCLUDED
#define RCFILE_H_INCLUDED

// maybe unnecessary
#include <sys/types.h>

#include "defs.h"

#define RCFILE ".seashellrc"

extern void run_rc_file(const pid_t seashell_pid);

#endif // RCFILE_H_INCLUDED
