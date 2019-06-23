#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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

bool parse_command(const char* line, struct command* cmd) {
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

bool run_command(const struct command cmd) {
  return true;
}
