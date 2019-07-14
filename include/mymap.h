#ifndef MYMAP_H_INCLUDED
#define MYMAP_H_INCLUDED

#define MAP_CAPACITY_HINT 64

typedef size_t (*hashfn)(void* data);
typedef bool (*equalityfn)(void* lhs, void* rhs);
typedef void (*cleanfn)(void *addr);

typedef struct {
  void** buckets;
  size_t cap;
  size_t valsz;
  size_t keysz;
  size_t size;
  hashfn hash;
  equalityfn valeq;
  equalityfn keyeq;
  cleanfn cleanval;
  cleanfn cleankey;
} map;

extern map map_new(size_t valsz, size_t keysz, size_t cap, hashfn hash,
		   equalityfn valeq, equalityfn keyeq, cleanfn cleanval, cleanfn cleankey);
extern map map_int_new(size_t valsz, size_t cap, equalityfn valeq, cleanfn cleanval);

#endif // MYMAP_H_INCLUDED
