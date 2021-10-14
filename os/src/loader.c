#include "config.h"
#include "log.h"
#include "string.h"
#include "task.h"
#include "trap.h"

struct KernelStack {
  uint8_t data[KERNEL_STACK_SIZE];
} __attribute__((aligned(4096)));

struct UserStack {
  uint8_t data[USER_STACK_SIZE];
} __attribute__((aligned(4096)));

static struct KernelStack KERNEL_STACK[MAX_APP_NUM];
static struct UserStack USER_STACK[MAX_APP_NUM];

uint64_t kernel_stack_get_sp(uint64_t app_id) {
  return (uint64_t)KERNEL_STACK[app_id].data + KERNEL_STACK_SIZE;
}

TaskContext *kernel_stack_push_context(TrapContext *trap_cx,
                                       TaskContext *task_cx, uint64_t app_id) {
  TrapContext *trap_cx_ptr =
      (TrapContext *)(kernel_stack_get_sp(app_id) - sizeof(TrapContext));
  *trap_cx_ptr = *trap_cx;
  TaskContext *task_cx_ptr = (TaskContext *)(trap_cx_ptr - sizeof(TaskContext));
  *task_cx_ptr = *task_cx;
  return task_cx_ptr;
}

uint64_t user_stack_get_sp(uint64_t app_id) {
  return (uint64_t)USER_STACK[app_id].data + USER_STACK_SIZE;
}

uint64_t loader_get_base_i(uint64_t app_id) {
  return APP_BASE_ADDRESS + app_id * APP_SIZE_LIMIT;
}

uint64_t loader_get_num_app() {
  extern uint64_t _num_app;
  return _num_app;
}

void loader_load_apps() {
  extern uint64_t _num_app;
  uint64_t *num_app_ptr = &_num_app;
  uint64_t num_app = loader_get_num_app();

  uint64_t app_start[MAX_APP_NUM + 1];
  for (uint64_t i = 0; i < num_app + 1; i++) {
    app_start[i] = num_app_ptr[i + 1];
  }

  // clear i-cache first
  asm volatile("fence.i");

  // load apps
  for (unsigned i = 0; i < num_app; i++) {
    uint64_t base_i = loader_get_base_i(i);

    // clear region
    memset((void *)base_i, 0, APP_SIZE_LIMIT);

    // load app from data section to memory
    uint8_t *src = (uint8_t *)app_start[i];
    uint8_t *dst = (uint8_t *)base_i;
    size_t len = app_start[i + 1] - app_start[i];
    memcpy(dst, src, len);

    info("App %d -> [0x%llx, 0x%llx)\n", i, base_i, base_i + len);
  }
}

TaskContext *loader_init_app_cx(uint64_t app_id) {
  return kernel_stack_push_context(
      app_init_context(
          loader_get_base_i(app_id), user_stack_get_sp(app_id),
          (TrapContext *)(kernel_stack_get_sp(app_id) - sizeof(TrapContext))),
      task_context_goto_restore((TaskContext *)(kernel_stack_get_sp(app_id) -
                                                sizeof(TrapContext) -
                                                sizeof(TaskContext))),
      app_id);
}
