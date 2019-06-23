#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "commands.h"
#include "builtins.h"

#define CHECK_ERROR(err, cmd) if (!err) { err = !(cmd); }

char error_msg[MAX_ERR_LEN] = {0};

bool read_line(char* line) {
  printf(PROMPT);
  if (fgets(line, MAX_CMD_LEN, stdin) == NULL) {
    strcpy(error_msg, "Could not read user input (fgets returned NULL)");
    return false;
  }
  bool succ = ends_with(line, "\n");
  if (succ) {
    int len = strlen(line);
    line[len-1] = '\0'; // get rid of newline
  } else {
    int len = MAX_CMD_LEN;
    while (strstr(line, "\n") == NULL) {
      fgets(line, MAX_CMD_LEN, stdin);
      len += strlen(line);
    }
    sprintf(error_msg, "User input was too long; it was %d chars, but only %d chars allowed",
	    len, MAX_CMD_LEN-1);
  }
  return succ;
}

int main(int argc, char *argv[]) {
  pid_t seashell_pid = getpid();

  char line[MAX_CMD_LEN];
  struct command cmd;
  pid_t child_pid = 0;
  bool error = false;
  bool is_builtin = false;
    
  while (true) {
    CHECK_ERROR(error, read_line(line));
    if (line[0] != '\0') {
      CHECK_ERROR(error, parse_command(line, &cmd));
      CHECK_ERROR(error, handle_builtin(cmd, &is_builtin));
      if (!is_builtin) {
	CHECK_ERROR(error, run_command(cmd, &child_pid));
      }
    } else continue;
    
    if (!error) {
      if (child_pid != 0) waitpid(child_pid, NULL, 0);
    } else {
      printf("ERROR: %s\n", error_msg);
      if (getpid() != seashell_pid) exit(0);
    }
  }

  printf("Exiting seashell\n");
  return 0;
}
