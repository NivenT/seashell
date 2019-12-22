#ifndef MYVEC_H_INCLUDED
#define MYVEC_H_INCLUDED

#define VECTOR_CAPACITY_HINT 16

typedef void (*CleanupElemFn)(void *addr);
typedef int (*CompareElemFn)(const void* lhs, const void* rhs);

// Should I use size_t instead of int?
typedef struct {
  void* data;
  int size;
  int cap;
  int elemsz;
  CleanupElemFn f;
} vec;

extern vec vec_new(int elemsz, int capacity_hint, CleanupElemFn f);
// The returned vector is really more like a non-owning "slice" or "view". Do not call free_vec on it
extern vec vec_tail(vec* v, int start);
extern void free_vec(vec* v);
extern int vec_size(const vec* v);
extern void vec_push(vec* v, void* elem);
extern void vec_pop(vec* v);
extern void* vec_get(const vec* v, int n);
extern void* vec_front(const vec* v);
extern void* vec_back(const vec* v);
extern void* vec_first(const vec* v);
extern void* vec_next(const vec* v, void* prev);
extern void vec_sort(vec* v, CompareElemFn comp);

#endif // MYVEC_H_INCLUDED
