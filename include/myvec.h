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

extern vec new_vec(int elemsz, int capacity_hint, CleanupElemFn f);
extern void free_vec(vec* v);
extern int vec_size(vec* v);
extern void vec_push(vec* v, void* elem);
extern void* vec_get(vec* v, int n);

#endif // MYVEC_H_INCLUDED
