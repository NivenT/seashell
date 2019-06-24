#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include <stdio.h>

#include "commands.h"

bool read_word(const char* str, const char** start, const char** end) {
  const char* ptr = str;
  while (*ptr != 0 && isspace(*ptr)) ++ptr;
  if (*ptr == 0) return false;

  *start = ptr;
  while (*ptr != 0 && !isspace(*ptr)) ++ptr;
  *end = ptr;

  return true;
}

int num_args(const command cmd) {
  int cnt = 0;
  while (cnt < MAX_NUM_ARGS && cmd.args[cnt]) cnt++;
  return cnt;
}

bool parse_command(const char* line, command* cmd) {
  const char *start = line, *end = 0;
  bool succ = read_word(line, &start, &end);
  if (!succ) {
    strcpy(error_msg, "Failed to read command");
    return false;
  }
  memcpy(cmd->name, start, end-start);
  cmd->name[end-start] = '\0';
  
  int idx = 0;
  line = end;
  while (read_word(line, &start, &end)) {
    //printf("%p (%c) - %p (%c) = %ld\n", end, *end, start, *start, end-start);

    if (idx >= MAX_NUM_ARGS) {
      sprintf(error_msg, "Too many arguments; only %d allowed", MAX_NUM_ARGS);
      return false;
    }
    
    cmd->args[idx] = malloc(end - start + 1);
    memcpy(cmd->args[idx], start, end-start);
    cmd->args[idx][end-start] = '\0';

    idx++;
    line = end;
  }
  cmd->args[idx] = 0;
  
  return true;
}

bool run_command(const command cmd, pid_t* pid) {
  *pid = fork();
  if (*pid == 0) {
    char* argv[MAX_NUM_ARGS + 2];
    command_to_argv(cmd, argv);
    
    execvp(argv[0], argv);
    strcpy(error_msg, "Command not found");
    return false;
  }
  return true;
}

void command_to_argv(const command cmd, char* argv[]) {
  argv[0] = &cmd.name[0];
  memcpy(&argv[1], cmd.args, MAX_NUM_ARGS);
  argv[MAX_NUM_ARGS+1] = 0;
}
