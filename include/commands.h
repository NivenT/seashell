#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#include <sys/types.h>

#include "defs.h"

typedef struct command command;

struct command {
  char name[MAX_CMD_NAME_LEN];
  // Should I turn these into strings?
  char* args[MAX_NUM_ARGS];
};

extern int num_args(const command cmd);

extern bool parse_command(const char* line, command* cmd);
extern bool run_command(const command cmd, pid_t* pid);

extern void command_to_argv(command cmd, char* argv[]);

#endif // COMMANDS_H_INCLUDED
