#include <stdlib.h>
#include <string.h>

#include "mymap.h"

typedef struct cell cell;

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

static size_t hash_char(void* data) {
  int x = *(char*)data;
  return hash_int(&x);
}

static bool eq_char(void* lhs, void* rhs) {
  return *(char*)lhs == *(char*)rhs;
}

map map_new(size_t valsz, size_t keysz, size_t nbuckets, hashfn hash,
	    equalityfn keyeq, cleanfn cleanval, cleanfn cleankey) {
  map ret;

  ret.nbuckets = nbuckets == 0 ? MAP_NBUCKETS_HINT : nbuckets;
  ret.valsz = valsz;
  ret.keysz = keysz;
  ret.size = 0;
  ret.hash = hash;
  ret.keyeq = keyeq;
  ret.cleanval = cleanval;
  ret.cleankey = cleankey;
  ret.buckets = calloc(ret.nbuckets, sizeof(cell*));

  return ret;
}

map map_int_new(size_t valsz, size_t nbuckets, cleanfn cleanval) {
  return map_new(valsz, sizeof(int), nbuckets, hash_int, eq_int, cleanval, NULL);
}

map map_char_new(size_t valsz, size_t nbuckets, cleanfn cleanval) {
  return map_new(valsz, sizeof(char), nbuckets, hash_char, eq_char, cleanval, NULL);
}

void* map_get(map* m, void* key) {
  size_t idx = m->hash(key)%m->nbuckets;
  for (cell* it = m->buckets[idx]; it; it = it->next) {
    if (m->keyeq(it->key, key)) return it->val;
  }
  return NULL;
}

void map_insert(map* m, void* key, void* val) {
  size_t idx = m->hash(key)%m->nbuckets;
  if (map_contains(m, key)) {
    for (cell* it = m->buckets[idx]; it; it = it->next) {
      if (m->keyeq(it->key, key)) {
	it->val = malloc(m->valsz);
	memcpy(it->val, val, m->valsz);
	return;
      }
    }
  } else {
    cell head;

    head.next = m->buckets[idx];
    head.key = malloc(m->keysz);
    memcpy(head.key, key, m->keysz);
    head.val = malloc(m->valsz);
    memcpy(head.val, val, m->valsz);
    
    m->buckets[idx] = malloc(sizeof(cell));
    memcpy(m->buckets[idx], &head, sizeof(cell));

    m->size++;
  }
}

// Note: can't do "return map_get(m, key) != NULL" since you might store NULL as a value
bool map_contains(map* m, void* key) {
  size_t idx = m->hash(key)%m->nbuckets;
  for (cell* it = m->buckets[idx]; it; it = it->next) {
    if (m->keyeq(it->key, key)) return true;
  }
  return false;
}

void map_remove(map* m, void* key) {
  size_t idx = m->hash(key)%m->nbuckets;
  for (cell** it = &m->buckets[idx]; *it; it = &(*it)->next) {
    if (m->keyeq((*it)->key, key)) {
      if (m->cleanval) m->cleanval((*it)->val);
      if (m->cleankey) m->cleankey((*it)->key);
      free((*it)->val);
      free((*it)->key);

      cell* trash = *it;
      *it = (*it)->next;

      free(trash);
      m->size--;
      return;
    }
  }
}

size_t map_size(map* m) {
  return m ? m->size : 0;
}

static cell* map_nonempty_bucket(map* m, int start) {
  if (!m) return NULL;
  for (int i = start; i < m->nbuckets; ++i) {
    if (m->buckets[i]) return m->buckets[i];
  }
  return NULL;
}

void* map_first(map* m) {
  cell* c = map_nonempty_bucket(m, 0);
  return c ? &c->key : NULL;
}

void* map_next(map* m, void* prev) {
  cell* c = (cell*)(prev - sizeof(cell*));
  if (c->next) return &c->next->key;
  size_t idx = m->hash(c->key)%m->nbuckets + 1;
  if (idx >= m->nbuckets) return NULL;
  c = map_nonempty_bucket(m, idx);
  return c ? &c->key : NULL;
}

static void free_cell(map* m, cell* c) {
  if (!c) return;
  free_cell(m, c->next);
  if (m->cleankey) m->cleankey(c->key);
  if (m->cleanval) m->cleanval(c->val);
  free(c->key);
  free(c->val);
  free(c);
}

void free_map(map* m) {
  if (!m) return;
  for (int i = 0; i < m->nbuckets; ++i) {
    free_cell(m, m->buckets[i]);
  }
  free(m->buckets);
  m->size = 0;
}
