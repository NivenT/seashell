#ifndef BUILTINS_H_INCLUDED
#define BUILTINS_H_INCLUDED

#include "pipeline.h"

extern const char* builtins[];

extern bool handle_builtin(command cmd, int idx);
extern bool is_builtin(const char* name, int* idx);

#endif // BUILTINS_H_INCLUDED
