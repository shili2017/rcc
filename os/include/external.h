#ifndef _EXTERNAL_H_
#define _EXTERNAL_H_

#include <stdint.h>

// buddy.c
void bd_init(void *, void *);
void bd_free(void *);
void *bd_malloc(uint64_t);

// list.c
struct list {
  struct list *next;
  struct list *prev;
};
void lst_init(struct list *);
void lst_remove(struct list *);
void lst_push(struct list *, void *);
void *lst_pop(struct list *);
void lst_print(struct list *);
int lst_empty(struct list *);

// vector.c
struct vector {
  uint64_t size, capacity, dsize;
  char *buffer;
};
void vector_new(struct vector *, uint64_t);
void vector_push(struct vector *, void *);
void vector_pop(struct vector *);
void *vector_back(struct vector *);
int vector_empty(struct vector *);
void vector_free(struct vector *);
void vector_remove(struct vector *, uint64_t);

// queue.c
struct queue {
  uint64_t size, front, tail, capacity, dsize;
  char *buffer;
};
void queue_new(struct queue *, uint64_t);
void queue_push(struct queue *, void *);
void queue_pop(struct queue *);
void *queue_front(struct queue *);
int queue_empty(struct queue *);
void queue_free(struct queue *);

// spinlock.c
struct spinlock {
  uint64_t locked; // Is the lock held?
  char *name;      // Name of lock.
};
void acquire(struct spinlock *);
int holding(struct spinlock *);
void initlock(struct spinlock *, char *);
void release(struct spinlock *);
void push_off(void);
void pop_off(void);
uint64_t sys_ntas(void);

#endif // _EXTERNAL_H_
