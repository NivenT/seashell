#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

#include "readline.h"
#include "builtins.h"
#include "utils.h"

#define COMPLETION_FUNC(type) static void complete_##type(const char* buf, linenoiseCompletions *lc)
#define HINTS_FUNC(type) static char* type##_hints(const char* buf, int* color, int* bold) 
#define CHECK_HINT(succ, hint) if (!succ) { succ = hint(buf, color, bold); }

static const int red = 31;
static const int green = 32;
static const int yellow = 33;
static const int blue = 34;
static const int magenta = 35;
static const int cyan = 36;
static const int white = 37;

char* history_file = NULL;

COMPLETION_FUNC(filenames) {
  const char* word = last_word(buf);
  DIR* dir = opendir(".");
  if (dir) {
    struct dirent* d;
    while ((d = readdir(dir)) != NULL) {
      if (starts_with(d->d_name, word)) {
	char* beg = strndup(buf, word-buf);
	char* full = concat(beg, d->d_name);
	linenoiseAddCompletion(lc, full);
	free(beg);
	free(full);
      }
    }
  }
  closedir(dir);
}

COMPLETION_FUNC(builtins) {
  for (int i = 0; builtins[i]; i++) {
    if (starts_with(builtins[i], buf)) linenoiseAddCompletion(lc, builtins[i]);
  }
}

HINTS_FUNC(builtins) {
  for (int i = 0; builtins[i]; i++) {
    int len = strlen(buf);
    if (len >= 3 && strncmp(builtins[i], buf, len) == 0) {
      *color = yellow;
      *bold = 0;
      // Not sure how to deal with the warning this generates
      // I could use strdup to make a copy, but then it may never get freed...
      return builtins[i] + len;
    }
  }
  return NULL;
}

static void completion(const char* buf, linenoiseCompletions *lc) {
  if (!buf) return;
  complete_filenames(buf, lc);
  complete_builtins(buf, lc);
}

static char* hints(const char* buf, int* color, int* bold) {
  char* ret = NULL;
  CHECK_HINT(ret, builtins_hints);
  return ret;
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

