#ifndef SUBPROCESS_H_INCLUDED
#define SUBPROCESS_H_INCLUDED

#include <sys/types.h>

#include "commands.h"

typedef struct subprocess subprocess;

struct subprocess {
  pid_t pid;
  int infd;
  int outfd;
};

extern bool spawn_subprocess(subprocess* sp, command cmd, bool send_input, bool get_output);
extern void free_subprocess(subprocess* sp);

#endif // SUBPROCESS_H_INCLUDED
