// ref: TestOS by Ziming Yuan (https://github.com/ZimingYuan/testos)

#include "external.h"
#include "log.h"
#include "string.h"

#define VECTOR_INIT_CAPACITY 8

void vector_new(struct vector *v, uint64_t dsize) {
  v->size = 0;
  v->capacity = VECTOR_INIT_CAPACITY;
  v->dsize = dsize;
  v->buffer = bd_malloc(v->capacity * v->dsize);
}

void vector_push(struct vector *v, void *d) {
  if (v->size == v->capacity) {
    v->capacity <<= 1;
    char *t = bd_malloc(v->capacity * v->dsize);
    memcpy(t, v->buffer, v->size * v->dsize);
    bd_free(v->buffer);
    v->buffer = t;
  }
  memcpy(v->buffer + (v->size++) * v->dsize, d, v->dsize);
}

void vector_pop(struct vector *v) {
  if (v->size == 0)
    panic("empty vector pop\n");
  v->size--;
}

void *vector_back(struct vector *v) {
  if (!v->size)
    panic("empty vector back\n");
  return v->buffer + (v->size - 1) * v->dsize;
}

int vector_empty(struct vector *v) { return !v->size; }

void vector_free(struct vector *v) { bd_free(v->buffer); }

void vector_remove(struct vector *v, uint64_t idx) {
  if (idx >= v->size)
    panic("invalid idx in vector remove\n");
  memmove(v->buffer + idx * v->dsize, v->buffer + (idx + 1) * v->dsize,
          (v->size - idx - 1) * v->dsize);
  v->size--;
}
