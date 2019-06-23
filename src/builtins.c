#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"

const char* builtins[] = {"exit", "quit", ""};

bool handle_builtin(const command cmd, bool* is_builtin) {
  int idx = -1;
  for (int i = 0; builtins[i]; i++) {
    if (strcmp(builtins[i], cmd.name) == 0) {
      idx = i;
      break;
    }
  }

  *is_builtin = true;
  switch(idx) {
  case 0: case 1: exit(0); break;
  default: *is_builtin = false; break;
  }
  
  return true;
}
