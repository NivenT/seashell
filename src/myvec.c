#include <stdlib.h>
#include <string.h>

#include "myvec.h"

// I should maybe stop assuming all these calls to malloc/realloc won't fail at some point

vec vec_new(int elemsz, int capacity_hint, CleanupElemFn f) {
  vec ret;

  ret.size = 0;
  ret.cap = capacity_hint == 0 ? VECTOR_CAPACITY_HINT : capacity_hint;
  ret.elemsz = elemsz;
  ret.f = f;
  ret.data = malloc(ret.cap * ret.elemsz);

  return ret;
}

vec vec_tail(vec* v, int start) {
  vec ret = { .data = vec_get(v, start), .size = v->size - start, .cap = v->cap - start,
	      .elemsz = v->elemsz, .f = NULL};
  return ret; 
}

void free_vec(vec* v) {
  if (!v) return;
  for (int i = 0; i < v->size; i++) {
    if (v->f) v->f(v->data + i*v->elemsz);
  }
  v->size = v->cap = v->elemsz = 0;
  free(v->data);
}

int vec_size(const vec* v) {
  return v ? v->size : 0;
}

void vec_push(vec* v, void* elem) {
  if (v->size >= v->cap) {
    v->cap <<= 1;
    v->data = realloc(v->data, v->cap * v->elemsz);
  }
  memcpy(vec_get(v, v->size++), elem, v->elemsz);
}

void vec_pop(vec* v) {
  if (v->size == 0) return;
  --v->size;
  if (v->f) v->f(v->data + v->size * v->elemsz);
}

void* vec_get(const vec* v, int n) {
  return v->data + (v->elemsz * n);
}

void* vec_front(const vec* v) {
  return v->size == 0 ? NULL : vec_get(v, 0);
}

void* vec_back(const vec* v) {
  return v->size == 0 ? NULL : vec_get(v, v->size - 1);
}

void* vec_first(const vec* v) {
  return v ? v->data : NULL;
}

void* vec_next(const vec* v, void* prev) {
  if (prev - v->data >= v->elemsz * (v->size - 1)) return NULL;
  return v && prev ? prev + v->elemsz : NULL;
}

void vec_sort(vec* v, CompareElemFn comp) {
  if (!v) return;
  qsort(v->data, v->size, v->elemsz, comp);
}
