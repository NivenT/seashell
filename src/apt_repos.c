#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

#include "apt_repos.h"
#include "subprocess.h"
#include "utils.h"

// TODO: Have something similar working for brew
// map from first letter -> vec of apts
static map apts = { .buckets = NULL };
// ^^^^^^^^^^^^^^^^^^^^^
// I chose to store things in a map instead of a vector to make searching through it faster
// However, when you think about it, "apt-cache search ." is gonna return commands in
// alphabetical order, so I could have just stored them in a vector and used binary search
// instead. Oh, well. Too late to change things.

static void cleanup_apts() {
  free_map(&apts);
}

static void clean_vec(void* addr) {
  if (addr) free_vec((vec*)addr);
}

static void clean_str(void* addr) {
  free(*(char**)addr);
}

// I'm not sure how I feel about this potentially blocking when you first hit tab
// I should maybe move it to its own thread
void init_apts() {
  apts = map_char_new(sizeof(vec), 32, clean_vec);

  command cmd = { .name = "apt-cache", .args = { "search", ".", NULL } };
  subprocess sp;
  if (spawn_subprocess(&sp, cmd, false, true)) {
    FILE* f = fdopen(sp.outfd, "r");
    if (f) {
      char line[MAX_CMD_LEN];
      while (fgets(line, MAX_CMD_LEN, f)) {
	char* apt = first_word(line);
	if (!apt) continue;
	if (!*apt) {
	  free(apt);
	  continue;
	}
	
	char c = *apt;
	if (map_contains(&apts, &c)) {
	  vec* v = (vec*)map_get(&apts, &c);
	  vec_push(v, &apt);
	} else {
	  vec v = vec_new(sizeof(char*), 1024, clean_str);
	  vec_push(&v, &apt);
	  map_insert(&apts, &c, &v);
	}	
      }
      fclose(f);
    }
    waitpid(sp.pid, NULL, 0);
    free_subprocess(&sp);
  }
  atexit(cleanup_apts);
}

const vec* apts_starting_with(char c) {
  return (vec*)map_get(&apts, &c);
}

vec apts_starting_with_sp(const char* prefix) {
  vec v = vec_new(sizeof(char*), 0, clean_str);

  subprocess sp;
  if (spawn_subprocess(&sp, (command){ .name = "apt-cache", .args = { "search", ".", NULL } },
		       false, true)) {
    FILE* f = fdopen(sp.outfd, "r");
    if (f) {
      char line[MAX_CMD_LEN];
      while (fgets(line, MAX_CMD_LEN, f)) {
	char* apt = first_word(line);
	if (starts_with(apt, prefix)) {
	  vec_push(&v, &apt);
	} else {
	  free(apt);
	}
      }
      fclose(f);
    }
    waitpid(sp.pid, NULL, 0);
    free_subprocess(&sp);
  }
  return v;
}
