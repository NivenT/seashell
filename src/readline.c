#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

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

  char* fldr;
  const char* file = rsplit(word, "/", &fldr);

  DIR* dir = opendir(!fldr ? "."  :
		     *fldr ? fldr : "/");
  if (dir) {
    struct dirent* d;
    while ((d = readdir(dir)) != NULL) {
      if (*d->d_name == '.') continue;
      if (starts_with(d->d_name, file)) {
	char* full = concat(buf, d->d_name + strlen(file));
	linenoiseAddCompletion(lc, full);
	free(full);
      }
    }
    if (fldr) free(fldr);
    closedir(dir);
  }
}

COMPLETION_FUNC(builtins) {
  for (int i = 0; builtins[i]; i++) {
    if (starts_with(builtins[i], buf)) linenoiseAddCompletion(lc, builtins[i]);
  }
}

COMPLETION_FUNC(commands) {
  if (strstr(buf, " ")) return;
  
  char* paths = getenv("PATH");
  if (!paths) paths = "/bin:/usr/bin";

  paths = strdup(paths); // make a copy since strsep modifies stuff
  char* iter = paths;
  for (char* path = strsep(&iter, ":"); path; path = strsep(&iter, ":")) {
    DIR* dir = opendir(path);
    if (dir) {
      struct dirent* d;
      while ((d = readdir(dir)) != NULL) {
	const char* strs[] = {path, "/", d->d_name, NULL};
	char* full_path = concat_many(strs);
	
	struct stat s;
	if (stat(full_path, &s) == 0 && (s.st_mode & S_IXUSR) && starts_with(d->d_name, buf)) {
	  linenoiseAddCompletion(lc, d->d_name);
	}
	free(full_path);
      }
      closedir(dir);
    }
  }
  free(paths);
}

HINTS_FUNC(builtins) {
  *color = yellow;
  *bold = 0;
  
  linenoiseSetFreeHintsCallback(NULL);
  for (int i = 0; builtins[i]; i++) {
    int len = strlen(buf);
    if (len >= 2 && strncmp(builtins[i], buf, len) == 0) {
      if (len < strlen(builtins[i])) {
	linenoiseSetFreeHintsCallback(free);
	return strdup(builtins[i] + len);
      } else switch(i) {
	case 2: return " <path>";
	case 5: return " <orig> <new>";
      }
    }
  }
  return NULL;
}

// Not sure how I feel about this
HINTS_FUNC(commands) {
  *color = yellow;
  *bold = 0;

  char* copy = strdup(buf);
  char* trimmed = trim(copy);
  if (strcmp(trimmed, "grep") == 0) {
    free(copy);
    return " <file> <pattern>";
  } else if (strcmp(trimmed, "ls") == 0) {
    free(copy);
    return " <path>";
  } else if (strcmp(trimmed, "git commit") == 0) {
    free(copy);
    return " -am <message>";
  } else if (strcmp(trimmed, "git clone") == 0) {
    free(copy);
    return " <repository>";
  } else if (strcmp(trimmed, "bookmark --save") == 0) {
    free(copy);
    return " <path> --name <name>";
  } else if (strcmp(trimmed, "bookmark --goto") == 0) {
    free(copy);
    return " <name>";
  } else if (strcmp(trimmed, "cat") == 0) {
    return " <file>";
  }
  free(copy);
  return NULL;
}

static void completion(const char* buf, linenoiseCompletions *lc) {
  if (!buf) return;
  complete_filenames(buf, lc);
  complete_builtins(buf, lc);
  complete_commands(buf, lc);
}

static char* hints(const char* buf, int* color, int* bold) {
  if (!buf) return NULL;
  char* ret = NULL;
  CHECK_HINT(ret, builtins_hints);
  CHECK_HINT(ret, commands_hints);
  if (ret && ends_with(buf, " ") && starts_with(ret, " ")) ret++;
  return ret;
}

static void cleanup() {
  free(history_file);
}

void init_linenoise() {
  linenoiseSetMultiLine(1);

  linenoiseSetCompletionCallback(completion);
  linenoiseSetHintsCallback(hints);

  const char* temp_strs[] = {home_dir, "/", LINENOISE_HISTORY_FILE, NULL};
  history_file = concat_many(temp_strs);
  atexit(cleanup);
  
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

  if (temp) linenoiseFree(temp);
  return succ;
}

