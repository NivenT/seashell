#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#include <sys/types.h>

#include "defs.h"

typedef struct command command;

struct command {
  char* name;
  // Should I turn these into strings?
  char* args[MAX_NUM_ARGS];
};

extern int num_args(const command cmd);
extern char* command_to_string(const command cmd); // mallocs
extern void free_cmd(command* cmd);

#endif // COMMANDS_H_INCLUDED
