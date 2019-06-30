#include <stdlib.h>
#include <string.h>

#include "myvec.h"

vec new_vec(int elemsz, int capacity_hint, CleanupElemFn f) {
  vec ret;

  ret.size = 0;
  ret.cap = capacity_hint == 0 ? VECTOR_CAPACITY_HINT : capacity_hint;
  ret.elemsz = elemsz;
  ret.f = f;
  ret.data = malloc(ret.cap * ret.elemsz);

  return ret;
}

void free_vec(vec* v) {
  if (!v) return;
  for (int i = 0; i < v->size; i++) {
    v->f(v->data + i*v->elemsz);
  }
  v->size = v->cap = v->elemsz = 0;
  free(v->data);
}

int vec_size(vec* v) {
  return v ? v->size : 0;
}

void vec_push(vec* v, void* elem) {
  if (v->size >= v->cap) {
    v->cap <<= 1;
    v->data = realloc(v->data, v->cap * v->elemsz);
  }
  memcpy(vec_get(v, v->size++), elem, v->elemsz);
}

void* vec_get(vec* v, int n) {
  return v->data + (v->elemsz * n);
}
