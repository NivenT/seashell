#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

#include "completions.h"
#include "hints.h"
#include "readline.h"
#include "builtins.h"
#include "utils.h"
#include "apt_repos.h"
#include "networking.h"

char* history_file = NULL;

static void cleanup() {
  free(history_file);
}

// TODO: Separate out hints/completions code into their own (2 new) files
void init_linenoise() {
  linenoiseSetMultiLine(1);

  linenoiseSetCompletionCallback(completion);
  linenoiseSetHintsCallback(hints);

  const char* temp_strs[] = {home_dir, "/", LINENOISE_HISTORY_FILE, NULL};
  history_file = concat_many(temp_strs);
  atexit(cleanup);

  linenoiseHistorySetMaxLen(LINENOISE_HISTORY_MAX_LEN);
  linenoiseHistoryLoad(history_file);

  init_completions();
  init_hints();
}

bool read_line(char* line) {
  char prompt[MAX_PROMPT_LEN];
  char cwd[MAX_PTH_LEN];
  
  getcwd(cwd, MAX_PTH_LEN);
  sprintf(prompt, PROMPT, cwd);
  
  char* temp = linenoise(prompt);
  int len = strlen(temp);

  bool succ = temp && len < MAX_CMD_LEN;
  if (succ) {
    strcpy(line, temp);
    if (line[0]) {
      linenoiseHistoryAdd(line);
      linenoiseHistorySave(history_file);
    }
  } else {
    line[0] = ' '; // Make line non-empty so the error msg gets printed
    sprintf(error_msg, "User input was too long; it was %d chars, but only %d chars allowed",
	    len, MAX_CMD_LEN-1);
  }

  if (temp) linenoiseFree(temp);
  return succ;
}

