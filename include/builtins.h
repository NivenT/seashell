#ifndef BUILTINS_H_INCLUDED
#define BUILTINS_H_INCLUDED

#include "pipeline.h"

extern const char* builtins[];

extern bool handle_builtin(pipeline* pipe, bool* is_builtin);

#endif // BUILTINS_H_INCLUDED
