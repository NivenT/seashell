#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#include <linenoise.h>

#include "builtins.h"
#include "bookmark.h"
#include "alias.h"
#include "utils.h"
#include "signals.h"
#include "job.h"

const char* builtins[] = {"exit", "quit", "cd", "bookmark", "home", "alias", "jobs", "%", "kill",
			  "history", "fg", "bg", NULL};
static char prev_dir[MAX_PTH_LEN] = {0};

static bool cd(const command cmd) {
  int count = num_args(cmd);
  if (count != 1) {
    sprintf(error_msg, "cd: too %s arguments; there should be exactly 1",
	    count > 1 ? "many" : "few");
    return false;
  }

  char cwd[MAX_PTH_LEN];
  getcwd(cwd, MAX_PTH_LEN);

  char dest[MAX_PTH_LEN];
  if (strcmp(cmd.args[0], "-") == 0) {
    if (!prev_dir[0]) {
      strcpy(error_msg, "No previous directory set");
      return false;
    }
    strcpy(dest, prev_dir);
  } else {
    strcpy(dest, cmd.args[0]);
  }
  
  if (chdir(dest) == 0) {
    strcpy(prev_dir, cwd);
    return true;
  } else {
    sprintf(error_msg, "cd: %s", strerror(errno));
    return false;
  }
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

static bool mykill(const command cmd, int outfd) {
  static const struct option options[] =
    {
     {"job", required_argument, NULL, 'j'},
     {"idx", required_argument, NULL, 'i'},
     {"pid", required_argument, NULL, 'p'},
     {"help", no_argument, NULL, 'h'}
    };

  char** argv = (char**)&cmd;
  int argc = num_args(cmd) + 1;

  char jobstr[MAX_NUM_LEN] = {0};
  char idxstr[MAX_NUM_LEN] = {0};
  char pidstr[MAX_NUM_LEN] = {0};
  bool help = false;
  
  opterr = 0;
  optind = 1;
  while(true) {
    char c = getopt_long(argc, argv, "j:i:p:h", options, NULL);
    if (c == -1) break;
    if (c == '?') continue;

    switch(c) {
    case 'j': strcpy(jobstr, optarg); break;
    case 'i': strcpy(idxstr, optarg); break;
    case 'p': strcpy(pidstr, optarg); break;
    case 'h': help = true; break;
    }
  }

  static const char* usage = "kill usage:\n\tkill --job ID --idx INDEX SIGNAL\n\tkill --pid PID SIGNAL";
  if (help) {
    dprintf(outfd, "%s\n", usage);
    return true;
  }
  
  char* sigstr = argv[argc-1];
  int sig = 0;

  // TODO: Use sys_siglist instead
  if (strcmp(sigstr, "SIGCONT") == 0) {
    sig = SIGCONT;
  } else if (strcmp(sigstr, "SIGTSTP") == 0) {
    sig = SIGTSTP;
  } else if (strcmp(sigstr, "SIGKILL") == 0) {
    sig = SIGKILL;
  } else {
    sprintf(error_msg, "kill: unrecognized signal %s", sigstr);
    return false;
  }

  if (pidstr[0]) {
    pid_t pid = strtoul(pidstr, NULL, 0);
    process* p = jl_get_proc(pid);
    if (p) kill(pid, sig);
  } else if (jobstr[0] && idxstr[0]) {
    size_t id = strtoul(jobstr, NULL, 0);
    size_t idx = strtoul(idxstr, NULL, 0);
    
    job* j = jl_get_job_by_id(id);
    if (j && vec_size(&j->processes) > idx) kill(((process*)vec_get(&j->processes, idx))->pid, sig);
  } else {
    strcpy(error_msg, usage);
    return false;
  }
  return true;
}

static bool fbg(const command cmd, bool fg, int outfd) {
  static const struct option options[] =
    {
     {"help", no_argument, NULL, 'h'}
    };

  char** argv = (char**)&cmd;
  int argc = num_args(cmd) + 1;

  char jobstr[MAX_NUM_LEN] = {0};
  bool help = false;
  
  opterr = 0;
  optind = 1;
  while(true) {
    char c = getopt_long(argc, argv, "h", options, NULL);
    if (c == -1) break;
    if (c == '?') continue;
    if (c == 'h') help = true;
  }

  const char* usage = fg ? "fg usage:\n\tfg JOB_ID\n\tfg --help" :
                           "bg usage:\n\tbg JOB_ID\n\tbg --help";
  if (help) {
    dprintf(outfd, "%s\n", usage);
    return true;
  }

  strcpy(jobstr, argv[argc-1]);
  job* j = NULL;
  if (jobstr[0]) {
    size_t id = strtoul(jobstr, NULL, 0);
    j = jl_get_job_by_id(id);
    if (!j) {
      sprintf(error_msg, "Could not find job with id %lu", id);
      return false;
    }
  } else {
    strcpy(error_msg, usage);
    return false;
  }
  return jl_resume(j, fg);
}


// TODO: Add --search flag
static bool history(struct command cmd, int outfd) {
  const char** hist = linenoiseHistoryGet();
  int len = linenoiseHistoryGetLen();
  for (int i = 0; i < len; i++) {
    dprintf(outfd, "%d\t%s\n", i, hist[i]);
  }
  return true;
}

bool handle_builtin(command cmd, int idx, int infd, int outfd) {
  bool ret = true;

  // Possibly excessive, but these are fast so it should be fine
  // Prevents me from forgetting to block in one of these functions
  sigset_t prev, block = get_sig_full();
  sigprocmask(SIG_BLOCK, &block, &prev);
  switch(idx) {
  case 0: case 1: exit(0); break;
  case 2: ret = cd(cmd); break;
  case 3: ret = bookmark(cmd); break;
  case 4: dprintf(outfd, "%s\n", home_dir); break;
  case 5: ret = alias(cmd); break;
  case 6: jl_print(); break;
  case 7: /* if (!jl_has_fg()) */ ret = jl_resume_first_stopped(); break;
  case 8: ret = mykill(cmd, outfd); break;
  case 9: ret = history(cmd, outfd); break;
  case 10: case 11: ret = fbg(cmd, idx == 10, outfd); break;
  }
  sigprocmask(SIG_SETMASK, &prev, NULL);
  
  return ret;
}

bool is_builtin(const char* name, int* idx) {
  *idx = -1;
  for (int i = 0; builtins[i]; i++) {
    if (strcmp(builtins[i], name) == 0) {
      *idx = i;
      return true;
    }
  }
  return false;
}
