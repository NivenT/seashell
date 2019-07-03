#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include <stdio.h>

#include "commands.h"

int num_args(const command cmd) {
  int cnt = 0;
  while (cnt < MAX_NUM_ARGS && cmd.args[cnt]) cnt++;
  return cnt;
}

void free_cmd(command* cmd) {
  if (cmd->name) free(cmd->name);
  for (int i = 0; i < MAX_NUM_ARGS && cmd->args[i]; i++) {
    free(cmd->args[i]);
  }
}
