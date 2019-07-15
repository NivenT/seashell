#include <assert.h>
#include <stdlib.h>

#include "tests.h"

void run_tests() {
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
  exit(0);
}
