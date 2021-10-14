#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>

#include "config.h"

struct TaskContext {
  uint64_t ra;
  uint64_t s[12];
};

typedef struct TaskContext TaskContext;

#define TaskStatusUnInit 0
#define TaskStatusReady 1
#define TaskStatusRunning 2
#define TaskStatusExited 3

struct TaskControlBlock {
  uint64_t task_cx_ptr;
  uint64_t task_status; // UnInit / Ready / Running / Exited
};

typedef struct TaskControlBlock TaskControlBlock;

struct TaskManager {
  uint64_t num_app;
  TaskControlBlock tasks[MAX_APP_NUM];
  uint64_t current_task;
};

typedef struct TaskManager TaskManager;

const uint64_t *get_task_cx_ptr2(TaskControlBlock *s);
TaskContext *task_context_goto_restore(TaskContext *c);

void task_run_first_task();
void task_run_next_task();
void task_mark_current_suspended();
void task_mark_current_exited();
void task_suspend_current_and_run_next();
void task_exit_current_and_run_next();

#endif // _TASK_H_
