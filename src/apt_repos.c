#include "apt_repos.h"

// TODO: Have something similar working for brew
// map from first letter -> vec of apts
static map apts = { .buckets = NULL };

static void cleanup_apts() {
  free_map(&apts);
}

// I'm not sure how I feel about this potentially blocking when you first hit tab
// I should maybe move it to its own thread
void init_apts() {
  apts = map_char_new(sizeof(vec), 32, free_vec);
  
  atexit(cleanup_apts);
}
