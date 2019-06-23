#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "defs.h"

int main(int argc, char *argv[]) {
  pid_t seashell_pid = getpid();
  printf(PROMPT);
  printf("\n");
  return 0;
}
