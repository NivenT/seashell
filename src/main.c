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
#include "apt_repos.h"
#include "networking.h"
#include "expression.h"

#define CHECK_ERROR(err, cmd) if (!err) { err = !(cmd); }

/* TODO List (in no particular order):
 * Maybe add && and ||
 * * Maybe add grouping symbols
 * Redirect output from/to other file descriptors
 * * e.g. "cmd 2> /tmp/errors" should redirect stderr (fd 2) to /tmp/errors
 * * e.g. "cmd 2> &1" should redirect stderr to stdout (fd 1)
 * Support environment variables
 * * e.g. "echo $HOME"
 * * e.g. "TZ=Pacific/Samoa date"
 * Subcommands
 * * e.g. "play $(ls songs | shuf)"
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
  } else if (!init_jobs()) {
    printf("Could not initialize job list: %s\n", strerror(errno));
    exit(0xBAD);
  }
  init_apts();
  init_networking();
}

static void wait_for_fg() {
  sigset_t prevmask = block_sig2(SIGCHLD, SIGUSR1);
  while (jl_has_fg() || expr_in_fg) sigsuspend(&prevmask);
  unblock_sig2(SIGCHLD, SIGUSR1, prevmask);
}

static void regain_terminal_control(const pid_t seashell_pid) {
  sigset_t prevmask = block_sig(SIGTTOU);
  if (tcsetpgrp(STDIN_FILENO, seashell_pid) != 0) {
    printf("Error: Could not regain control of the terminal\n");
  }
  unblock_sig(SIGTTOU, prevmask);
}

void run_line(char line[MAX_CMD_LEN], const pid_t seashell_pid, bool error) {
  expression expr;

  line = trim(line); // It bothers me that this works
  if (line[0] != '\0') {
    CHECK_ERROR(error, apply_aliases(line));
    vec tkns = parse_string(line);
    CHECK_ERROR(error, build_expression(&tkns, &expr));
    free_vec(&tkns);
    CHECK_ERROR(error, execute_expression(&expr));
  } else return;

  if (!error) {
    wait_for_fg();
    free_expression(&expr);
    regain_terminal_control(seashell_pid);
  } else {
    dprintf(STDERR_FILENO, "ERROR: %s\n", error_msg);
    if (getpid() != seashell_pid) exit(0xBAD);
  }
}

int main(int argc, char *argv[]) {
  const pid_t seashell_pid = getpid();
  
  atexit(cleanup);
  init_globals();
  //run_tests();
  run_rc_file(seashell_pid);

  while (true) {
    char line[MAX_CMD_LEN];
    bool error = false;
    
    CHECK_ERROR(error, read_line(line));
    run_line(line, seashell_pid, error);
  }
  return 0;
}
