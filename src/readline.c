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

#define COMPLETION_FUNC(type) static void complete_##type(const char* buf, linenoiseCompletions *lc)
#define HINTS_FUNC(type) static char* type##_hints(const char* buf, int* color, int* bold) 
#define CHECK_HINT(succ, hint, buf) if (!succ) { succ = hint(buf, color, bold); }

static const int red = 31;
static const int green = 32;
static const int yellow = 33;
static const int blue = 34;
static const int magenta = 35;
static const int cyan = 36;
static const int white = 37;

char* history_file = NULL;
char* full_buf = NULL;
const char* post_cursor = NULL;

/*
static void add_completion_helper(linenoiseCompletions* lc, const char* complete) {
  char* safe_space = replace_all(complete, " ", "\\ ");
  linenoiseAddCompletion(lc, safe_space);
  free(safe_space);
}
*/

static void add_completion(linenoiseCompletions* lc, const char* complete) {
  char* comp = concat(complete, post_cursor);
  if (full_buf && *full_buf) {
    const char* strs[] = {full_buf, "| ", comp, NULL};
    char* full = concat_many(strs);
    linenoiseAddCompletion(lc, full);
    free(full);
  } else {
    linenoiseAddCompletion(lc, comp);
  }
  free(comp);
}

COMPLETION_FUNC(filenames) {
  const char* word = last_word(buf);

  char* fldr;
  const char* file = rsplit(word, "/", &fldr);

  if (fldr && *fldr == '~') {
    if (fldr[1] == '\0' || fldr[1] == '/') {
      char* temp = concat(home_dir, fldr + 1);
      free(fldr);
      fldr = temp;
    }
  }

  DIR* dir = opendir(!fldr ? "."  :
		     !*fldr ? "/" : fldr);
  if (dir) {
    struct dirent* d;
    while ((d = readdir(dir)) != NULL) {
      if (strcmp(d->d_name, ".")*strcmp(d->d_name, "..") == 0) continue;
      if (starts_with(d->d_name, file)) {
	char* safe = replace_all(d->d_name + strlen(file), " ", "\\ ");
	char* full = concat(buf, safe);
	add_completion(lc, full);
	free(full);
	free(safe);
      }
    }
    closedir(dir);
  }
  if (fldr) free(fldr);
}

COMPLETION_FUNC(builtins) {
  for (int i = 0; builtins[i]; i++) {
    if (starts_with(builtins[i], buf)) add_completion(lc, builtins[i]);
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
	if (starts_with(d->d_name, buf)) {
	  const char* strs[] = {path, "/", d->d_name, NULL};
	  char* full_path = concat_many(strs);

	  struct stat s;
	  if (stat(full_path, &s) == 0 && (s.st_mode & S_IXUSR)) {
	    add_completion(lc, d->d_name);
	  }
	  free(full_path);
	}
      }
      closedir(dir);
    }
  }
  free(paths);
}

COMPLETION_FUNC(common) {
  // Should these be stored in an external .seashell_completions file?
  static const char* common_cmds[] =
    {
     "sudo apt-get install",
     "sudo apt-get remove",
     "apt-cache search",
     "brew install",
     "cargo build --release",
     "cargo run --release",
     "curl -sSL -X POST -H \"Content-Type: application/x-www-form-urlencoded\"",
     "curl -sSL -X GET -H \"Content-Type: application/json\"",
     "git push origin master",
     "git push --set-upstream origin",
     "git remote add origin",
     "git remote get-url",
     "git remote set-url",
     "git remote rename",
     "git remote remove",
     "git commit -m",
     "git pull",
     "git clone https://github.com/",
     "git clone git@github.com:",
     "git rebase -i",
     "git stash",
     "git diff --stat",
     "git status",
     "git add",
     "git grep",
     "git checkout -b",
     "git branch -D",
     "git ls-files",
     "git diff",
     "git stash",
     "git merge",
     "git submodule update --recursive --init",
     "git submodule add",
     "git fetch",
     NULL
    };

  // TODO: Make this code not trash
  char* inp = strdup(buf);
  char* tinp = trim(inp);

  int ilen = strlen(tinp);
  for (int i = 0; common_cmds[i]; ++i) {
    char* cmd = strdup(common_cmds[i]);
    int clen = strlen(cmd);
    
    int coffset = 0;
    int ioffset = 0;
    while (ioffset < ilen && coffset < clen) {
      char* cword = first_word(cmd + coffset);
      char* iword = first_word(tinp + ioffset);

      int iwlen = strlen(iword);
      int iclen = strlen(cword);
      
      if (strcmp(cword, iword) != 0) {
	if (ioffset + iwlen == ilen && starts_with(cword, iword)) {
	  char* temp = concat(tinp, cword + iwlen);
	  add_completion(lc, temp);
	  free(temp);
	} else {
	  free(cword);
	  free(iword);
	  break;
	}
      }

      coffset += iclen + 1;
      ioffset += iwlen;
      
      free(cword);
      free(iword);

      iword = tinp + ioffset;
      while (*iword && isspace(*iword)) {
	++iword;
	++ioffset;
      }
    }
    
    free(cmd);
  }
  free(inp);
}

COMPLETION_FUNC(apt_cache_search) {
}

HINTS_FUNC(builtins) {  
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
	case 8: return " --job <id> --idx <idx> <sig>";
	case 10: case 11: return " <job_idx>";
      }
    }
  }
  return NULL;
}

// Not sure how I feel about this
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

static void completion(const char* buf, linenoiseCompletions *lc) {
  if (!buf) return;

  char* beg = strndup(buf, lc->cursor - buf);
  post_cursor = lc->cursor;
  
  const char* post_pipe = rsplit(beg, "|", &full_buf);
  while (*post_pipe && isspace(*post_pipe)) ++post_pipe;
  
  complete_filenames(post_pipe, lc);
  complete_builtins(post_pipe, lc);
  complete_commands(post_pipe, lc);
  complete_common(post_pipe, lc);
  complete_apt_cache_search(post_pipe, lc);
  
  free(full_buf);
  free(beg);
}

static char* hints(const char* buf, int* color, int* bold) {
  if (!buf) return NULL;
  const char* post_pipe = rsplit(buf, "|", NULL);
  
  char* ret = NULL;
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

void init_linenoise() {
  linenoiseSetMultiLine(1);

  linenoiseSetCompletionCallback(completion);
  linenoiseSetHintsCallback(hints);

  const char* temp_strs[] = {home_dir, "/", LINENOISE_HISTORY_FILE, NULL};
  history_file = concat_many(temp_strs);
  atexit(cleanup);

  linenoiseHistorySetMaxLen(LINENOISE_HISTORY_MAX_LEN);
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

