#include "string.h"
#include "task.h"

TaskManager TASK_MANAGER;

void task_manager_init() {
  memset(TASK_MANAGER.ready_queue, 0,
         MAX_TASK_NUM * sizeof(TaskControlBlock *));
}

bool task_manager_almost_full() {
  return (TASK_MANAGER.tail - TASK_MANAGER.head >= MAX_TASK_NUM - 1);
}

int64_t task_manager_add_task(TaskControlBlock *task) {
  if (TASK_MANAGER.tail - TASK_MANAGER.head == MAX_TASK_NUM) {
    return -1;
  }
  TASK_MANAGER.ready_queue[TASK_MANAGER.tail % MAX_TASK_NUM] = task;
  TASK_MANAGER.tail++;
  return 0;
}

TaskControlBlock *task_manager_fetch_task() {
  if (TASK_MANAGER.head == TASK_MANAGER.tail) {
    return NULL;
  }
  TaskControlBlock *task =
      TASK_MANAGER.ready_queue[TASK_MANAGER.head % MAX_TASK_NUM];
  TASK_MANAGER.head++;
  return task;
}

TaskControlBlock *task_manager_fetch_task_by_pid(uint64_t pid) {
  if (pid == processor_current_task()->pid) {
    return processor_current_task();
  }
  for (uint64_t i = TASK_MANAGER.head; i < TASK_MANAGER.tail; i++) {
    if (TASK_MANAGER.ready_queue[i % MAX_TASK_NUM]->pid == pid) {
      return TASK_MANAGER.ready_queue[i % MAX_TASK_NUM];
    }
  }
  return NULL;
}
