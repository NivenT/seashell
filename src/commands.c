#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include <stdio.h>

#include "commands.h"
#include "utils.h"

int num_args(const command cmd) {
  int cnt = 0;
  while (cnt < MAX_NUM_ARGS && cmd.args[cnt]) cnt++;
  return cnt;
}

// This could be written better
char* command_to_string(const command cmd) {
  int end = num_args(cmd);
  char* temp = join((const char**)&cmd.args[0], (const char**)&cmd.args[end], " ");
  const char* strs[] = {cmd.name, " ", temp, NULL};
  char* ret = concat_many(strs);
  free(temp);
  return ret;
}

void free_cmd(command* cmd) {
  if (cmd->name) free(cmd->name);
  for (int i = 0; i < MAX_NUM_ARGS && cmd->args[i]; i++) {
    free(cmd->args[i]);
  }
}
