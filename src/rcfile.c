#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "rcfile.h"
#include "utils.h"

extern void run_line(char line[MAX_CMD_LEN], const pid_t seashell_pid, bool error);

static bool create_rc_file(char* file) {
  static const char* default_rc_contents =
    "alias ls \"ls --color=auto\"\nalias ll \"ls --color=auto -alF\"\nalias grep \"grep --color=auto\"";
  int fd = open(file, O_RDWR | O_CREAT | O_APPEND, 0644);
  if (fd == -1) return false;
  bool succ = write_all(fd, default_rc_contents, strlen(default_rc_contents));
  close(fd);
  return succ;
}

void run_rc_file(const pid_t seashell_pid) {
  const char* temp_strs[] = {home_dir, "/", RCFILE, NULL};
  char* rc_file = concat_many(temp_strs);
  
  if (access(rc_file, F_OK) == -1 && !create_rc_file(rc_file)) {
    free(rc_file);
    return;
  }

  int fd = open(rc_file, O_RDONLY);
  if (fd == -1) return;
  FILE* f = fdopen(fd, "r");
  if (!f) {
    close(fd);
    return;
  }
  char line[MAX_CMD_LEN];
  while (fgets(line, MAX_CMD_LEN, f)) {
    run_line(line, seashell_pid, false);
  }
  fclose(f);
  close(fd);
  free(rc_file);
}
