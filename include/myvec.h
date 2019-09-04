#ifndef MYVEC_H_INCLUDED
#define MYVEC_H_INCLUDED

#define VECTOR_CAPACITY_HINT 16

typedef void (*CleanupElemFn)(void *addr);

typedef struct {
  void* data;
  int size;
  int cap;
  int elemsz;
  CleanupElemFn f;
} vec;

extern vec vec_new(int elemsz, int capacity_hint, CleanupElemFn f);
extern void free_vec(vec* v);
extern int vec_size(const vec* v);
extern void vec_push(vec* v, void* elem);
extern void vec_pop(vec* v);
extern void* vec_get(const vec* v, int n);
extern void* vec_first(vec* v);
extern void* vec_next(vec* v, void* prev);

#endif // MYVEC_H_INCLUDED
