#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "signals.h"
#include "job.h"

static void handleSIGCHLD(int sig) {
  int status;
  while (true) {
    pid_t pid = waitpid(-1, &status, WNOHANG | WCONTINUED | WUNTRACED);
    if (pid <= 0) return;

    sigset_t prevmask = block_sig(SIGCHLD); // Is this necessary?
    if (WIFEXITED(status)) {
      jl_update_state(pid, TERMINATED);
    } else if (WIFSTOPPED(status)) {
      jl_update_state(pid, STOPPED);
    } else if (WIFCONTINUED(status)) {
      jl_update_state(pid, RUNNING);
    } else if (WIFSIGNALED(status)) {
      jl_update_state(pid, TERMINATED);
    }
    unblock_sig(SIGCHLD, prevmask);
  }
}

static void handleSIGINT(int sig) {
  sigset_t prevmask = block_sig(SIGCHLD);
  if (jl_has_fg()) {
    pid_t gpid = jl_fg_gpid();
    kill(-gpid, SIGINT);
    jl_update_state(gpid, TERMINATED);
  } else {
    exit(0);
  }
  unblock_sig(SIGCHLD, prevmask);
}

static void handleSIGTSTP(int sig) {
  sigset_t prevmask = block_sig(SIGCHLD);
  if (jl_has_fg()) {
    pid_t gpid = jl_fg_gpid();
    kill(-gpid, SIGTSTP);
    jl_update_state(gpid, STOPPED);
  } else {
    exit(0);
  }
  unblock_sig(SIGCHLD, prevmask);
}

// I've never used a function pointer without a typedef before, and I see why
static bool install_signal_handler(int signum, void (*sighandler)(int)) {
  struct sigaction action;
  action.sa_handler = sighandler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_RESTART;
  return sigaction(signum, &action, NULL) == 0;
}

bool install_signal_handlers() {
  bool ret = true;
  ret = ret && install_signal_handler(SIGCHLD, handleSIGCHLD);
  ret = ret && install_signal_handler(SIGINT, handleSIGINT);
  ret = ret && install_signal_handler(SIGTSTP, handleSIGTSTP);
  return ret;
}

sigset_t get_sig_singleton(int sig) {
  sigset_t singleton;
  sigemptyset(&singleton);
  sigaddset(&singleton, sig);
  return singleton;
}

sigset_t get_sig_full() {
  sigset_t all;
  sigfillset(&all);
  return all;
}

sigset_t block_sig(int sig) {
  sigset_t old, block = get_sig_singleton(sig);
  sigprocmask(SIG_BLOCK, &block, &old);
  return old;
}

void unblock_sig(int sig, sigset_t prevmask) {
  sigset_t block = get_sig_singleton(sig);
  // assume sigismember won't error
  if (!sigismember(&prevmask, sig)) sigprocmask(SIG_UNBLOCK, &block, NULL);
}
