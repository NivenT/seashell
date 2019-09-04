#include <unistd.h>

#include "subprocess.h"
#include "utils.h"

// Should this set error_msg?
bool spawn_subprocess(subprocess* sp, command cmd, bool send_input, bool get_output) {
  if (!sp) return false;
  *sp = (subprocess){ .pid = 0, .infd = -1, .outfd = -1 }; // Does this do what I hope?

  int infds[2], outfds[2];
  if (send_input) {
    if (pipe(infds) < 0) return false;
    sp->infd = infds[1];
  }
  if (get_output) {
    if (pipe(outfds) < 0) return false;
    sp->outfd = outfds[0];
  }

  sp->pid = fork();
  if (sp->pid < 0) return false;
  else if (sp->pid == 0) {
    if (send_input) {
      if (dup2(infds[0], STDIN_FILENO) < 0) return false;
      closeall(infds, 2);
    }
    if (get_output) {
      if (dup2(outfds[1], STDOUT_FILENO) < 0) return false;
      closeall(outfds, 2);
    }
    execvp(cmd.name, (char**)&cmd);
    return false;
  }
  if (send_input) close(infds[0]);
  if (get_output) close(outfds[1]);
  return true;
}

void free_subprocess(subprocess* sp) {
  if (!sp) return;
  if (sp->infd >= 0) close(sp->infd);
  if (sp->outfd >= 0) close(sp->outfd);
  *sp = (subprocess){ .pid = 0, .infd = -1, .outfd = -1 };
}
