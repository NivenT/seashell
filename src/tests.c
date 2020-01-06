#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tests.h"
#include "utils.h"
#include "networking.h"

static void test_map() {
  map primes = map_int_new(sizeof(size_t), 0, NULL);
  assert(map_size(&primes) == 0);

  int not_key = 10;
  assert(!map_contains(&primes, &not_key));
  not_key = 3;
  assert(!map_get(&primes, &not_key));

  int keys[] = {1, 2, 3, 5, 7, 11, 13};
  size_t vals[] = {2, 3, 5, 11, 17, 31, 41};

  for (int i = 0; i < sizeof(keys)/sizeof(int); i++) {
    map_insert(&primes, &keys[i], &vals[i]);
  }

  not_key = 4;
  assert(!map_contains(&primes, &not_key));
  assert(map_contains(&primes, &keys[3]));
  assert(*(size_t*)map_get(&primes, &keys[3]) == vals[3]);
  assert(map_size(&primes) == sizeof(keys)/sizeof(int));

  map_remove(&primes, &keys[2]);
  map_remove(&primes, &keys[3]);

  assert(map_size(&primes) == sizeof(keys)/sizeof(int) - 2);
  assert(!map_contains(&primes, &keys[2]));
  assert(!map_contains(&primes, &keys[3]));
  assert(map_contains(&primes, &keys[4]));
  
  free_map(&primes);
}

static void test_replace_all() {
  const char* tests[][4] =
    {
     {"this doesn't work", "doesn't", "does", "this does work"},
     {"make spaces safe", " ", "\\ ", "make\\ spaces\\ safe"},
     {"redredredred", "red", "blue", "blueblueblueblue"},
     {NULL, "here", "there", NULL},
     {"here", NULL, "there", NULL},
     {"here", "here", NULL, NULL},
     {"here", "here", "", ""},
     {"s*t*r*i***n*g****", "*", "", "string"},
     NULL
    };
  for (int i = 0; tests[i] && (tests[i][0] || tests[i][1] || tests[i][2]); i++) {
    char* new = replace_all(tests[i][0], tests[i][1], tests[i][2]);
    assert(new == tests[i][3] || strcmp(new, tests[i][3]) == 0);
    if (new) free(new);
  }
}

static void test_embedded_file() {
  char contents[] = "Test \"Passed\"\n!\n";
  assert(strcmp(contents, "Test \"Passed\"\n!\n") == 0);
}

void run_tests() {
  test_map();
  test_replace_all();
  test_embedded_file();
  exit(0);
}
