#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>

#include "config.h"
#include "mm.h"
#include "trap.h"

typedef struct {
  uint64_t ra;
  uint64_t s[12];
} TaskContext;

#define TASK_STATUS_READY 0
#define TASK_STATUS_RUNNING 1
#define TASK_STATUS_EXITED 2

#define BIG_STRIDE 100000
#define MAX_PRIORITY 32
#define DEFAULT_PRIORITY 16

typedef uint64_t TaskStatus;

typedef struct {
  TaskContext *task_cx_ptr;
  TaskStatus task_status;
  MemorySet memory_set;
  PhysPageNum trap_cx_ppn;
  uint64_t base_size;

  // stride scheduling
  uint64_t priority;
  uint64_t stride;
} TaskControlBlock;

typedef struct {
  TaskControlBlock tasks[MAX_APP_NUM];
  uint64_t current_task;
  uint64_t num_app;
} TaskManager;

// task.c
void task_init();
void task_run_first_task();
void task_run_next_task();
void task_mark_current_suspended();
void task_mark_current_exited();
void task_suspend_current_and_run_next();
void task_exit_current_and_run_next();
uint64_t task_get_current_task();
void task_set_priority(int64_t prio);
uint64_t task_current_user_token();
TrapContext *task_current_trap_cx();

// task_control_block.c
const TaskContext **get_task_cx_ptr2(TaskControlBlock *s);
TrapContext *get_trap_cx(TaskControlBlock *s);
uint64_t get_user_token(TaskControlBlock *s);
void task_control_block_new(uint8_t *elf_data, size_t elf_size, uint64_t app_id,
                            TaskControlBlock *s);
void task_control_block_free(TaskControlBlock *s);

// task_manager.c
void task_manager_init();
void task_manager_run_first_task();
void task_manager_mark_current_suspended();
void task_manager_mark_current_exited();
int64_t task_manager_find_next_task();
void task_manager_run_next_task();
uint64_t task_manager_get_current_task();
void task_manager_set_priority(int64_t prio);
uint64_t task_manager_get_current_token();
TrapContext *task_manager_get_current_trap_cx();

// task_context.c
void task_context_goto_trap_return(TaskContext *cx);

#endif // _TASK_H_
