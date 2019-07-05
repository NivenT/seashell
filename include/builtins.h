#ifndef BUILTINS_H_INCLUDED
#define BUILTINS_H_INCLUDED

#include "commands.h"

extern const char* builtins[];

extern bool handle_builtin(command cmd, bool* is_builtin);

#endif // BUILTINS_H_INCLUDED
