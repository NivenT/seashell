#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "alias.h"

vec aliases;

static void free_alias(void* data) {
  char** strs = (char**)data;
  if (*strs) free(*(strs++));
  if (*strs) free(*strs);
}

static void cleanup() {
  free_vec(&aliases);
}

void init_aliases() {
  aliases = vec_new(2*sizeof(char*), 0, free_alias);
  atexit(cleanup);
}

bool alias(const command cmd) {
  int count = num_args(cmd);
  if (count != 2) {
    sprintf(error_msg, "cd: too %s arguments; there should be exactly 2",
	    count > 2 ? "many" : "few");
    return false;
  }

  char* pair[2] = {strdup(cmd.args[0]), strdup(cmd.args[1])};
  vec_push(&aliases, &pair);
  return true;
}

// This is gonna be annoying to fix once I add environment variable support
bool apply_aliases(char line[MAX_CMD_LEN]) {
  int len = strlen(line);
  int size = vec_size(&aliases);

  char* curr = line;
  do {
    char* word = first_word(curr);
    for (int i = 0; i < size; i++) {
      char** strs = (char**)vec_get(&aliases, i);
      if (strcmp(word, strs[0]) == 0) {
	int len2 = strlen(word);
	int len3 = strlen(strs[1]);
	if (len + (len3 - len2) >= MAX_CMD_LEN) {
	  sprintf(error_msg,
		  "Could not apply alias ('%s' -> '%s') because it would make the command too long",
		  strs[0], strs[1]);
	  free(word);
	  return false;
	}
      
	memmove(&curr[len3], &curr[len2], len - len2 + 1);
	memcpy(&curr[0], strs[1], len3);
	break;
      }
    }
    free(word);
    curr = strstr(curr, "|");
    if (curr) curr = trim(curr+1);
  } while (curr);
  return true;
}
