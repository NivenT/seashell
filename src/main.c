#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "commands.h"

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
  bool error = false;
  while (true) {
    error = false;

    CHECK_ERROR(error, read_line(line));
    struct command cmd;
    CHECK_ERROR(error, parse_command(line, &cmd));
    CHECK_ERROR(error, run_command(cmd));
    
    if (!error) {
      //printf("%s (%ld)\n", line, strlen(line));
      printf("%s", cmd.name);
      for (int i = 0; i < MAX_NUM_ARGS; i++) {
	if (cmd.args[i] == NULL) break;
	printf(" '%s'", cmd.args[i]);
      }
      printf("\n");
    } else {
      printf("ERROR: %s\n", error_msg);
    }
  }

  printf("Exiting seashell\n");
  return 0;
}
