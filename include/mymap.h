#ifndef MYMAP_H_INCLUDED
#define MYMAP_H_INCLUDED

#define MAP_NBUCKETS_HINT 64

#include <stdbool.h>
#include <stddef.h>

typedef size_t (*hashfn)(void* data);
typedef bool (*equalityfn)(void* lhs, void* rhs);
typedef void (*cleanfn)(void *addr);

struct cell {
  struct cell* next;
  void* key;
  void* val;
};

typedef struct {
  struct cell** buckets;
  size_t nbuckets;
  size_t valsz;
  size_t keysz;
  size_t size;
  hashfn hash;
  equalityfn keyeq;
  cleanfn cleanval;
  cleanfn cleankey;
} map;

extern map map_new(size_t valsz, size_t keysz, size_t nbuckets, hashfn hash,
		   equalityfn keyeq, cleanfn cleanval, cleanfn cleankey);
extern map map_int_new(size_t valsz, size_t nbuckets, cleanfn cleanval);
extern void* map_get(map* m, void* key);
extern void map_insert(map* m, void* key, void* val);
extern bool map_contains(map* m, void* key);
extern void map_remove(map* m, void* key);
extern size_t map_size(map* m);
extern void free_map(map* m);

#endif // MYMAP_H_INCLUDED
