#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

#include "apt_repos.h"
#include "subprocess.h"
#include "utils.h"

// TODO: Have something similar working for brew
// map from first letter -> vec of apts
static map apts = { .buckets = NULL };

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

  
  
  exit(0);
}
