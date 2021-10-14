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
    TASK_MANAGER.tasks[i].task_status = TaskStatusReady;
    info("App %d -> TaskControlBlock: [0x%llx, 0x%llx)\n", i,
         &(TASK_MANAGER.tasks[i]),
         &(TASK_MANAGER.tasks[i]) + sizeof(TaskControlBlock));
  }
  TASK_MANAGER.current_task = 0;
}

void task_manager_run_first_task() {
  TASK_MANAGER.tasks[0].task_status = TaskStatusRunning;
  const TaskContext **next_task_cx_ptr2 =
      get_task_cx_ptr2(&(TASK_MANAGER.tasks[0]));
  uint64_t _unused = 0;
  __switch((const TaskContext **)&_unused, next_task_cx_ptr2);
}

void task_manager_mark_current_suspended() {
  uint64_t current = TASK_MANAGER.current_task;
  TASK_MANAGER.tasks[current].task_status = TaskStatusReady;
}

void task_manager_mark_current_exited() {
  uint64_t current = TASK_MANAGER.current_task;
  TASK_MANAGER.tasks[current].task_status = TaskStatusExited;
}

int64_t task_manager_find_next_task() {
  uint64_t current = TASK_MANAGER.current_task;
  uint64_t num_app = TASK_MANAGER.num_app;
  for (int64_t i = current + 1; i < current + num_app + 1; i++) {
    int64_t id = i % num_app;
    if (TASK_MANAGER.tasks[id].task_status == TaskStatusReady) {
      return id;
    }
  }
  return -1;
}

void task_manager_run_next_task() {
  int64_t next = task_manager_find_next_task();
  if (next >= 0) {
    uint64_t current = TASK_MANAGER.current_task;
    TASK_MANAGER.tasks[next].task_status = TaskStatusRunning;
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
