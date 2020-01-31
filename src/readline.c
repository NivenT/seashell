#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

#include "readline.h"
#include "builtins.h"
#include "utils.h"
#include "apt_repos.h"
#include "networking.h"

#define HINTS_FUNC(type) static char* type##_hints(const char* buf, int* color, int* bold) 
#define CHECK_HINT(succ, hint, buf) if (!succ) { succ = hint(buf, color, bold); }

static const int red __attribute__((unused)) = 31;
static const int green __attribute__((unused)) = 32;
static const int yellow __attribute__((unused)) = 33;
static const int blue __attribute__((unused)) = 34;
static const int magenta __attribute__((unused)) = 35;
static const int cyan __attribute__((unused)) = 36;
static const int white __attribute__((unused)) = 37;

char* history_file = NULL;

HINTS_FUNC(builtins) {  
  linenoiseSetFreeHintsCallback(NULL);
  for (int i = 0; builtins[i]; i++) {
    int len = strlen(buf);
    if (len >= HINTS_MIN_LEN && strncmp(builtins[i], buf, len) == 0) {
      if (len < strlen(builtins[i])) {
	linenoiseSetFreeHintsCallback(free);
	return strdup(builtins[i] + len);
      } else switch(i) {
	case 2: return " <path>";
	case 5: return " <orig> <new>";
	case 8: return " --job <id> --idx <idx> <sig>";
	case 10: case 11: return " <job_idx>";
      }
    }
  }
  return NULL;
}

HINTS_FUNC(commands) {
  // Should these be stored in an external .seashell_hints file and have regex support?
  static char* const hints[][2] =
    {
     {"grep", " <file> <pattern>"},
     {"ls", " <path>"},
     {"rm", " [<files...> | -r <folders...>]"},
     {"git commit", " -m <message>"},
     {"git clone", " <repository>"},
     {"git submodule add", " <repository>"},
     {"git merge", " <branch>"},
     {"git diff --", "stat"},
     {"bookmark --save", " <path> --name <name>"},
     {"bookmark --goto", " <name>"},
     {"cat", " <file>"},
     {"tar", " [-cf (compress) | -tvf (list) | -xf (extract)]"},
     {"tar -cf", " <archive> <files...>"},
     {"tar -tvf", " <archive>"},
     {"tar -xf", " <archive>"},
     {"emacs -nw", " <file>"},
     {"ssh", " <user>@<destination>"},
     {"xargs", " <command>"},
     {"ps", " aux"},
     {"curl", " -sSL -X <method> -H <header> -d <data> -D <response header file> <url>"},
     {"curl -sSL", " -X <method> -H <header> -d <data> -D <response header file> <url>"},
     {"curl -sSL -X POST", " -H \"Content-Type: application/x-www-form-urlencoded\" -d <data> -D <response header file> <url>"},
     {"curl -sSL -X GET", " -H \"Content-Type: application/json\" -D <response header file> <url>"},
     NULL
    };
  
  char* copy = strdup(buf);
  char* trimmed = trim(copy);

  char* ret = NULL;
  for (int i = 0; hints[i][0] || hints[i][1]; i++) {
    if (strcmp(trimmed, hints[i][0]) == 0) {
      ret = hints[i][1];
      break;
    }
  }
  free(copy);
  return ret;
}

static int comp_str(const void* lhs, const void* rhs) {
  const char* l = *(const char**)lhs;
  const char* r = *(const char**)rhs;
  return strcmp(l, r);
}

// Return negative if lhs comes before rhs
typedef struct { char* str; int freq; } record;
static int comp_rec(const void* lhs, const void* rhs) {
  const record l = *(const record*)lhs;
  const record r = *(const record*)rhs;
  int diff = r.freq - l.freq;
  return diff == 0 ? strlen(r.str) - strlen(l.str) : diff;
}

HINTS_FUNC(history) {
  int buf_len = strlen(buf);
  if (buf_len < HINTS_MIN_LEN) return NULL;
  vec hist = vec_new(sizeof(char*), 0, NULL);

  const char** hist_ptrs = linenoiseHistoryGet();
  int len = linenoiseHistoryGetLen();
  for (int i = 0; i < len; i++) {
    if (strlen(hist_ptrs[i]) >= HIST_HINT_MIN_LEN) vec_push(&hist, &hist_ptrs[i]);
  }
  vec_sort(&hist, comp_str);

  vec recs = vec_new(sizeof(record), 0, NULL);
  for (int i = 0; i < vec_size(&hist);) {
    record rec = { .str = *(char**)vec_get(&hist, i), .freq = 0 };
    while (strcmp(*(char**)vec_get(&hist, i++), rec.str) == 0 && i < vec_size(&hist)) rec.freq++;
    if (rec.freq >= HIST_HINT_MIN_FREQ) vec_push(&recs, &rec);
  }
  vec_sort(&recs, comp_rec);

  char* ret = NULL;
  len = vec_size(&recs) < HIST_HINT_NUM_RECS ? vec_size(&recs) : HIST_HINT_NUM_RECS;
  for (int i = 0; i < len; i++) {
    char* str = ((record*)vec_get(&recs, i))->str;
    if (starts_with(str, buf)) {
      ret = str + buf_len;
      break;
    }
  }
  free_vec(&hist);
  free_vec(&recs);
  return ret;
}

static char* hints(const char* buf, int* color, int* bold) {
  if (!buf) return NULL;
  const char* post_pipe = rsplit(buf, "|", NULL);
  
  char* ret = NULL;
  CHECK_HINT(ret,  history_hints, buf);
  CHECK_HINT(ret, builtins_hints, post_pipe);
  CHECK_HINT(ret, commands_hints, post_pipe);
  while (ret && ends_with(buf, " ") && starts_with(ret, " ")) ret++;

  *color = yellow;
  *bold = 0;
  return ret;
}

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

