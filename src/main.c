#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "utils.h"
#include "commands.h"
#include "builtins.h"
#include "parser.h"
#include "pipeline.h"

#define CHECK_ERROR(err, cmd) if (!err) { err = !(cmd); }

char error_msg[MAX_ERR_LEN] = {0};
char* home_dir = NULL;
/*
bool read_line_custom(char* line) {
  char cwd[MAX_PTH_LEN];
  getcwd(cwd, MAX_PTH_LEN);
  
  printf(PROMPT, cwd);
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
*/
bool read_line(char* line) {
  char prompt[MAX_PROMPT_LEN];
  char cwd[MAX_PTH_LEN];
  
  getcwd(cwd, MAX_PTH_LEN);
  sprintf(prompt, PROMPT, cwd);
  
  char* temp = readline(prompt);
  int len = strlen(temp);

  bool succ = temp && len < MAX_CMD_LEN;
  if (succ) {
    strcpy(line, temp);
    if (line[0]) add_history(line);
  } else {
    line[0] = ' '; // Make line non-empty so the error msg gets printed
    sprintf(error_msg, "User input was too long; it was %d chars, but only %d chars allowed",
	    len, MAX_CMD_LEN-1);
  }

  if (temp) free(temp);
  return succ;
}

// TODO: Don't run from children
void cleanup() {
  rl_clear_history();
}

int main(int argc, char *argv[]) {
  atexit(cleanup);
  rl_bind_key('\t', rl_complete);
  pid_t seashell_pid = getpid();

  if ((home_dir = getenv("HOME")) == NULL) {
    home_dir = getpwuid(getuid())->pw_dir;
  }
  
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
