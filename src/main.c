#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>

#include "utils.h"
#include "commands.h"
#include "builtins.h"
#include "parser.h"
#include "pipeline.h"
#include "readline.h"
#include "alias.h"
#include "rcfile.h"
#include "signals.h"
#include "job.h"
#include "tests.h"

#define CHECK_ERROR(err, cmd) if (!err) { err = !(cmd); }

/* TODO List (in order):
 * Actually do systems-y things
 * * add background processes
 * * add kill builtin
 * * add input/output files
 * Support single quoted strings in input (e.g. 'like this' instead of just "like this")
 * Support comments in input (e.g. ls # blah)
 * add history builtin
 */

/* Maybe do sometime:
 * Have tab completion take into account the location of the cursor (jk. This would (will?) require forking linenoise and surfacing this information myself)
 */

char error_msg[MAX_ERR_LEN] = {0};
char* home_dir = NULL;

static void cleanup() {
}

static void init_globals() {
  if ((home_dir = getenv("HOME")) == NULL) {
    home_dir = getpwuid(getuid())->pw_dir;
  }

  init_linenoise();
  init_aliases();
  if (!install_signal_handlers()) {
    printf("Could not install custom signal handlers: %s\n", strerror(errno));
    exit(0xBAD);
  }
  init_jobs();
}

void run_line(char line[MAX_CMD_LEN], const pid_t seashell_pid, bool error) {
  pid_t child_pid = 0;
  pipeline pipe;
  bool is_builtin;

  
  line = trim(line); // It bothers me that this works
  if (line[0] != '\0') {
    CHECK_ERROR(error, apply_aliases(line));
    vec tkns = parse_string(line);
    CHECK_ERROR(error, build_pipeline(tkns, &pipe));
    free_vec(&tkns);
    CHECK_ERROR(error, handle_builtin(pipe.cmd, &is_builtin));
    if (!is_builtin) {
      job* j = jl_new_job(true);
      CHECK_ERROR(error, execute_pipeline(pipe, &child_pid, j));
    }
    free_pipeline(&pipe);
  } else return;
  
  if (!error) {
    sigset_t prevmask = block_sig(SIGCHLD);
    while (jl_has_fg()) sigsuspend(&prevmask);
    unblock_sig(SIGCHLD, prevmask);
  } else {
    printf("ERROR: %s\n", error_msg);
    if (getpid() != seashell_pid) exit(0xBAD);
  }
}

int main(int argc, char *argv[]) {
  const pid_t seashell_pid = getpid();
  
  //run_tests();
  atexit(cleanup);
  init_globals();
  run_rc_file(seashell_pid);
  
  while (true) {
    char line[MAX_CMD_LEN];
    bool error = false;
    
    CHECK_ERROR(error, read_line(line));
    run_line(line, seashell_pid, error);
  }
  return 0;
}
