#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "utils.h"
#include "commands.h"
#include "builtins.h"
#include "parser.h"
#include "pipeline.h"
#include "readline.h"

#define CHECK_ERROR(err, cmd) if (!err) { err = !(cmd); }

char error_msg[MAX_ERR_LEN] = {0};
char* home_dir = NULL;

static void test_some_func() {
  char* tests[][2] =
    {
     {"foldr/file", "/"},
     {"dir/foldr/file", "/"},
     {"one#two#three#four", "#"},
     {"thisandthatandthisandmore", "and"},
     {"blah.txt", "/"},
     {"", "hello"},
     {NULL, "sep"},
     {"str", NULL},
     {"end/", "/"},
     {"/", "/"},
     {"//", "/"},
     NULL
    };
  for (int i = 0; tests[i] && (tests[i][0] || tests[i][1]); i++) {
    char* before;
    const char* after = rsplit(tests[i][0],  tests[i][1], &before);
    printf("rsplit(%s, %s) -> %s, %s\n", tests[i][0], tests[i][1], before, after);
    if (i == 0) {
      after = rsplit(tests[i][0],  tests[i][1], NULL);
      printf("rsplit(%s, %s, (null)) -> %s\n", tests[i][0], tests[i][1], after);
    }
    if (before) free(before);
  }
  exit(0);
}

static void cleanup() {
  if (history_file) free(history_file);
}

int main(int argc, char *argv[]) {
  //test_some_func();
  atexit(cleanup);
  
  pid_t seashell_pid = getpid();
  if ((home_dir = getenv("HOME")) == NULL) {
    home_dir = getpwuid(getuid())->pw_dir;
  }

  init_linenoise();
  while (true) {
    char line[MAX_CMD_LEN];
    command cmd = {0, {0}};
    pid_t child_pid = 0;
    bool error = false;
    bool is_builtin = false;
    pipeline pipe;
    
    CHECK_ERROR(error, read_line(line));
    if (line[0] != '\0') {
      vec tkns = parse_string(line);
      CHECK_ERROR(error, build_pipeline(tkns, &pipe));
      free_vec(&tkns);
      CHECK_ERROR(error, execute_pipeline(pipe, &child_pid));
      free_pipeline(&pipe);
    } else continue;
    
    if (!error) {
      if (child_pid != 0) waitpid(child_pid, NULL, 0);
    } else {
      printf("ERROR: %s\n", error_msg);
      if (getpid() != seashell_pid) exit(0xBAD);
    }
  }
  return 0;
}
