#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "builtins.h"

const char* builtins[] = {"exit", "quit", "cd", ""};

bool cd(const command cmd) {
  if (num_args(cmd) != 1) {
    strcpy(error_msg, "cd: too many arguments");
    return false;
  }

  if (chdir(cmd.args[0]) == 0) return true;
  switch(errno) {
  case EACCES: strcpy(error_msg, "cd: permission denied"); break;
  case EFAULT: strcpy(error_msg, "cd: path points outside accessible address space"); break;
  case EIO: strcpy(error_msg, "cd: IO error"); break;
  case ELOOP: strcpy(error_msg, "cd: too many symlinks"); break;
  case ENAMETOOLONG: strcpy(error_msg, "cd: path too long"); break;
  case ENOENT: strcpy(error_msg, "cd: file does not exist"); break;
  case ENOTDIR: strcpy(error_msg, "cd: part of path is not a directory"); break;
  }
  return false;
}

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
  case 2: return cd(cmd); break;
  default: *is_builtin = false; break;
  }
  
  return true;
}
