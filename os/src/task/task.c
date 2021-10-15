#include "task.h"
#include "loader.h"
#include "log.h"

extern void __switch(const TaskContext **current_task_cx_ptr2,
                     const TaskContext **next_task_cx_ptr2);

TaskManager TASK_MANAGER;

void task_manager_init() {
  TASK_MANAGER.num_app = loader_get_num_app();
  for (unsigned i = 0; i < TASK_MANAGER.num_app; i++) {
    TASK_MANAGER.tasks[i].task_cx_ptr = loader_init_app_cx(i);
    TASK_MANAGER.tasks[i].task_status = TASK_STATUS_READY;
    TASK_MANAGER.tasks[i].priority = DEFAULT_PRIORITY;
    TASK_MANAGER.tasks[i].stride = 0;
  }
  TASK_MANAGER.current_task = 0;
}

void task_manager_run_first_task() {
  TASK_MANAGER.tasks[0].task_status = TASK_STATUS_RUNNING;
  const TaskContext **next_task_cx_ptr2 =
      get_task_cx_ptr2(&(TASK_MANAGER.tasks[0]));
  uint64_t _unused = 0;
  __switch((const TaskContext **)&_unused, next_task_cx_ptr2);
}

void task_manager_mark_current_suspended() {
  uint64_t current = TASK_MANAGER.current_task;
  TASK_MANAGER.tasks[current].task_status = TASK_STATUS_READY;
}

void task_manager_mark_current_exited() {
  uint64_t current = TASK_MANAGER.current_task;
  TASK_MANAGER.tasks[current].task_status = TASK_STATUS_EXITED;
}

int64_t task_manager_find_next_task() {
  uint64_t current = TASK_MANAGER.current_task;
  uint64_t num_app = TASK_MANAGER.num_app;

  // stride scheduling
  // todo: fix stride overflow
  uint64_t min_stride = __UINT64_MAX__;
  uint64_t min_id = -1;
  for (uint64_t i = 0; i < num_app; i++) {
    if (TASK_MANAGER.tasks[i].task_status == TASK_STATUS_READY) {
      uint64_t current_stride = TASK_MANAGER.tasks[i].stride;
      if (current_stride < min_stride) {
        min_stride = current_stride;
        min_id = i;
      }
    }
  }
  return min_id;
}

void task_manager_run_next_task() {
  int64_t next = task_manager_find_next_task();
  if (next >= 0) {
    uint64_t current = TASK_MANAGER.current_task;
    TASK_MANAGER.tasks[next].task_status = TASK_STATUS_RUNNING;

    // stride scheduling
    uint64_t pass = BIG_STRIDE / TASK_MANAGER.tasks[next].priority;
    TASK_MANAGER.tasks[next].stride += pass;

    TASK_MANAGER.current_task = next;
    const TaskContext **current_task_cx_ptr2 =
        get_task_cx_ptr2(&(TASK_MANAGER.tasks[current]));
    const TaskContext **next_task_cx_ptr2 =
        get_task_cx_ptr2(&(TASK_MANAGER.tasks[next]));
    __switch(current_task_cx_ptr2, next_task_cx_ptr2);
  } else {
    panic("All applications completed!\n");
  }
}

uint64_t task_manager_get_current_task() { return TASK_MANAGER.current_task; }

void task_manager_set_priority(int64_t prio) {
  if (prio < 2) {
    prio = 2;
  } else if (prio > MAX_PRIORITY) {
    prio = MAX_PRIORITY;
  }
  uint64_t current = TASK_MANAGER.current_task;
  TASK_MANAGER.tasks[current].priority = prio;
}

void task_init() { task_manager_init(); }

void task_run_first_task() { task_manager_run_first_task(); }

void task_run_next_task() { task_manager_run_next_task(); }

void task_mark_current_suspended() { task_manager_mark_current_suspended(); }

void task_mark_current_exited() { task_manager_mark_current_exited(); }

void task_suspend_current_and_run_next() {
  task_mark_current_suspended();
  task_run_next_task();
}

void task_exit_current_and_run_next() {
  task_mark_current_exited();
  task_run_next_task();
}

uint64_t task_get_current_task() { return task_manager_get_current_task(); }

void task_set_priority(int64_t prio) { task_manager_set_priority(prio); }
