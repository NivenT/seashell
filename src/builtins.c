#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#include "builtins.h"
#include "bookmark.h"
#include "alias.h"
#include "utils.h"
#include "signals.h"
#include "job.h"

const char* builtins[] = {"exit", "quit", "cd", "bookmark", "home", "alias", "jobs", "%", NULL};

static bool cd(const command cmd) {
  int count = num_args(cmd);
  if (count != 1) {
    sprintf(error_msg, "cd: too %s arguments; there should be exactly 1",
	    count > 1 ? "many" : "few");
    return false;
  }

  if (chdir(cmd.args[0]) == 0) return true;
  sprintf(error_msg, "cd: %s", strerror(errno));
  return false;
}

static bool bookmark(const command cmd) {
  static const struct option options[] =
    {
     {"save", required_argument, NULL, 's'},
     {"name", required_argument, NULL, 'n'},
     {"goto", required_argument, NULL, 'g'},
     {"list", no_argument, NULL, 'l'}
    };

  char** argv = (char**)&cmd;
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

bool handle_builtin(pipeline* pipe, bool* is_builtin) {
  if (vec_size(&pipe->cmds) == 0) return true;
  command cmd = *(command*)vec_get(&pipe->cmds, 0);
  
  int idx = -1;
  for (int i = 0; builtins[i]; i++) {
    if (strcmp(builtins[i], cmd.name) == 0) {
      idx = i;
      break;
    }
  }

  *is_builtin = true;
  bool ret = true;

  // Possibly excessive, but these are fast so it should be fine
  // Prevents me from forgetting to block in one of these functions
  sigset_t prev, block = get_sig_full();
  sigprocmask(SIG_BLOCK, &block, &prev);
  switch(idx) {
  case 0: case 1: free_pipeline(pipe); exit(0); break;
  case 2: ret = cd(cmd); break;
  case 3: ret = bookmark(cmd); break;
  case 4: printf("%s\n", home_dir); break;
  case 5: ret = alias(cmd); break;
  case 6: jl_print(); break;
  case 7: if (!jl_has_fg()) jl_resume_first_stopped(); break;
  default: *is_builtin = false; break;
  }
  sigprocmask(SIG_SETMASK, &prev, NULL);
  
  return ret;
}
