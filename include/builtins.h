#ifndef BUILTINS_H_INCLUDED
#define BUILTINS_H_INCLUDED

#include "pipeline.h"

extern const char* builtins[];

// This first argument only exists to be able to cleanup memory on exit
// I'm not a fan of having to include it
extern bool handle_builtin(pipeline* pipe, command cmd, int idx, int infd, int outfd);
extern bool is_builtin(const char* name, int* idx);

#endif // BUILTINS_H_INCLUDED
