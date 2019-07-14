#include "mymap.h"

// https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
static size_t hash_int(void* data) {
  unsigned int x = *(int*)data;
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = (x >> 16) ^ x;
  return x;
}

static bool eq_int(void* lhs, void* rhs) {
  return *(int*)lhs == *(int*)rhs;
}

static void nop(void* addr) {}

map map_new(size_t valsz, size_t keysz, size_t cap, hashfn hash,
	    equalityfn valeq, equalityfn keyeq, cleanfn cleanval, cleanfn cleankey) {
  map ret;

  ret.cap = cap == 0 ? MAP_CAPACITY_HINT : cap;
  ret.valsz = valsz;
  ret.keysz = keysz;
  ret.size = 0;
  ret.hash = hash;
  ret.valeq = valeq;
  ret.keyeq = keyeq;
  ret.cleanval = cleanval;
  ret.cleankey = cleankey;
  ret.buckets = calloc(ret.cap, sizeof(void*));

  return ret;
}

map map_int_new(size_t valsz, size_t cap, equalityfn valeq, cleanfn cleanval) {
  return map_new(valsz, sizeof(int), cap, hash_int, valeq, eq_int, cleanval, nop);
}
