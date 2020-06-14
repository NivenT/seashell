#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "jumper.h"
#include "job.h"

static void jumper() {
}

bool play_game(char* cmd_str) {
  job* j = jl_get_foreground();
  if (!j) {
    strcpy(error_msg, "Cannot run 'jumper' in the background");
    return false;
  }
  pid_t pid = fork();
  if (pid == 0) {
    //raise(SIGTSTP);
    jumper();
    exit(0);
  } else {
    job_add_process(j, pid, RUNNING, cmd_str);
    return true;
  }
}
