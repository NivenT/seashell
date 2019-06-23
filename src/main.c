#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

bool read_line(char* line) {
  printf(PROMPT);
  if (fgets(line, MAX_CMD_LEN, stdin) == NULL) return false;
  bool succ = ends_with(line, "\n");
  if (succ) {
    int len = strlen(line);
    line[len-1] = '\0'; // get rid of newline
  } else while (strstr(line, "\n") == NULL) {
      fgets(line, MAX_CMD_LEN, stdin);
  }
  return succ;
}

int main(int argc, char *argv[]) {
  pid_t seashell_pid = getpid();

  char line[MAX_CMD_LEN];
  while (true) {
    if (read_line(line)) {
      printf("%s (%ld)\n", line, strlen(line));
    } else {
      printf("sad\n");
    }
  }

  printf("Exiting seashell\n");
  return 0;
}
