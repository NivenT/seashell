#ifndef BUILTINS_H_INCLUDED
#define BUILTINS_H_INCLUDED

#include "commands.h"

struct expression;

extern const char* builtins[];

// I am a big fan of removing the comment that was hear before
extern bool handle_builtin(command cmd, int idx, int infd, int outfd);
extern bool is_builtin(const char* name, int* idx);

#endif // BUILTINS_H_INCLUDED
