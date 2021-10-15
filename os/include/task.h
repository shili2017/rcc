#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>

#include "config.h"

struct TaskContext {
  uint64_t ra;
  uint64_t s[12];
};

typedef struct TaskContext TaskContext;

#define TASK_STATUS_UNINIT 0
#define TASK_STATUS_READY 1
#define TASK_STATUS_RUNNING 2
#define TASK_STATUS_EXITED 3

#define BIG_STRIDE 100000
#define MAX_PRIORITY 32
#define DEFAULT_PRIORITY 16

struct TaskControlBlock {
  TaskContext *task_cx_ptr;
  uint64_t task_status; // UnInit / Ready / Running / Exited

  // stride scheduling
  uint64_t priority;
  uint64_t stride;
};

typedef struct TaskControlBlock TaskControlBlock;

struct TaskManager {
  TaskControlBlock tasks[MAX_APP_NUM];
  uint64_t current_task;
  uint64_t num_app;
};

typedef struct TaskManager TaskManager;

const TaskContext **get_task_cx_ptr2(TaskControlBlock *s);
TaskContext *task_context_goto_restore(TaskContext *c);

void task_init();
void task_run_first_task();
void task_run_next_task();
void task_mark_current_suspended();
void task_mark_current_exited();
void task_suspend_current_and_run_next();
void task_exit_current_and_run_next();
uint64_t task_get_current_task();
void task_set_priority(int64_t prio);

#endif // _TASK_H_
