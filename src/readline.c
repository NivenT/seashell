#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "readline.h"
#include "utils.h"

char* history_file = NULL;

static void completion(const char* buf, linenoiseCompletions *lc) {
  if (!buf) return;
  if (buf[0] == 'h') {
    linenoiseAddCompletion(lc, "hello");
    linenoiseAddCompletion(lc, "hello World");
  }
}

static char* hints(const char* buf, int* color, int* bold) {
  if (strcasecmp(buf, "hello") == 0) {
    *color = 35; // what color is this
    *bold = 0;
    return " World";
  }
  return NULL;
}

void init_linenoise() {
  linenoiseSetCompletionCallback(completion);
  linenoiseSetHintsCallback(hints);

  const char* temp_strs[] = {home_dir, "/", LINENOISE_HISTORY_FILE, NULL};
  history_file = concat_many(temp_strs);

  linenoiseHistoryLoad(history_file);
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

  if (temp) free(temp);
  return succ;
}

