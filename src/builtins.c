#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#include "builtins.h"
#include "bookmark.h"
#include "utils.h"

static const char* builtins[] = {"exit", "quit", "cd", "bookmark", "home", ""};

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

bool bookmark(const command cmd) {
  static const struct option options[] =
    {
     {"save", required_argument, NULL, 's'},
     {"name", required_argument, NULL, 'n'},
     {"goto", required_argument, NULL, 'g'},
     {"list", no_argument, NULL, 'l'}
    };

  char* argv[MAX_NUM_ARGS+2];
  command_to_argv(cmd, argv);
  int argc = num_args(cmd) + 1;

  char save_path[MAX_PTH_LEN] = {0};
  char name[MAX_PTH_LEN] = {0};
  char goto_name[MAX_PTH_LEN] = {0};
  bool list = false;
  
  opterr = 0;
  optind = 1;
  while(true) {
    char c = getopt_long(argc, argv, "s:n:g:l", options, NULL);
    if (c == -1) break;
    if (c == '?') continue;

    switch(c) {
    case 's': strcpy(save_path, optarg); break;
    case 'n': strcpy(name, optarg); break;
    case 'g': strcpy(goto_name, optarg); break;
    case 'l': list = true; break;
    }
  }

  if (list) return list_bookmarks();
  else if (save_path[0]) return save_bookmark(save_path, name);
  else if (goto_name[0]) return goto_bookmark(goto_name);
  else return true;
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
  case 3: return bookmark(cmd); break;
  case 4: printf("%s\n", home_dir); break;
  default: *is_builtin = false; break;
  }
  
  return true;
}
