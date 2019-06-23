#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#include "defs.h"

struct command {
  char name[MAX_CMD_NAME_LEN];
  char* args[MAX_NUM_ARGS];
};

extern bool parse_command(const char* line, struct command* cmd);
extern bool run_command(const struct command cmd);

#endif // COMMANDS_H_INCLUDED
