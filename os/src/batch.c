#include "log.h"
#include "string.h"
#include "trap.h"

#define KERNEL_STACK_SIZE (4096 * 2)
#define USER_STACK_SIZE (4096 * 2)
#define MAX_APP_NUM 16
#define APP_BASE_ADDRESS 0x80400000
#define APP_SIZE_LIMIT 0x20000

struct KernelStack {
  uint8_t data[KERNEL_STACK_SIZE];
};

struct UserStack {
  uint8_t data[USER_STACK_SIZE];
};

static struct KernelStack KERNEL_STACK;
static struct UserStack USER_STACK;

uint64_t kernel_stack_get_sp() {
  return (uint64_t)KERNEL_STACK.data + KERNEL_STACK_SIZE;
}

TrapContext *kernel_stack_push_context(TrapContext *c) {
  TrapContext *context_ptr =
      (TrapContext *)(kernel_stack_get_sp() - sizeof(TrapContext));
  *context_ptr = *c;
  return context_ptr;
}

uint64_t user_stack_get_sp() {
  return (uint64_t)USER_STACK.data + USER_STACK_SIZE;
}

struct AppManager {
  uint64_t num_app;
  uint64_t current_app;
  uint64_t app_start[MAX_APP_NUM + 1];
};

static struct AppManager APP_MANAGER;

void app_manager_init() {
  extern uint64_t _num_app;
  uint64_t *num_app_ptr = &_num_app;
  APP_MANAGER.num_app = *num_app_ptr;

  APP_MANAGER.current_app = 0;

  uint64_t *app_start = APP_MANAGER.app_start;
  for (uint64_t i = 0; i < APP_MANAGER.num_app + 1; i++) {
    app_start[i] = num_app_ptr[i + 1];
  }
}

void app_manager_print_app_info() {
  info("num_app = %lld\n", APP_MANAGER.num_app);

  uint64_t *app_start = APP_MANAGER.app_start;
  for (uint64_t i = 0; i < APP_MANAGER.num_app; i++) {
    info("app_%lld [%llx, %llx)\n", i, app_start[i], app_start[i + 1]);
  }
}

void app_manager_load_app(uint64_t app_id) {
  if (app_id >= APP_MANAGER.num_app) {
    panic("All applications completed!\n");
  }
  info("Loading app_%lld\n", app_id);

  // clear icache
  asm volatile("fence.i");

  // clear app area
  for (uint8_t *i = (uint8_t *)APP_BASE_ADDRESS;
       i < (uint8_t *)(APP_BASE_ADDRESS + APP_SIZE_LIMIT); i++) {
    *i = 0;
  }

  uint64_t *app_start = APP_MANAGER.app_start;

  uint8_t *app_src = (uint8_t *)app_start[app_id];
  uint8_t *app_dst = (uint8_t *)APP_BASE_ADDRESS;
  uint64_t app_len = app_start[app_id + 1] - app_start[app_id];

  memcpy(app_dst, app_src, app_len);
}

uint64_t app_manager_get_current_app() { return APP_MANAGER.current_app; }

void app_manager_move_to_next_app() { APP_MANAGER.current_app += 1; }

void batch_init() {
  app_manager_init();
  app_manager_print_app_info();
}

extern void __restore(uint64_t);

void batch_run_next_app() {
  uint64_t current_app = app_manager_get_current_app();
  app_manager_load_app(current_app);
  app_manager_move_to_next_app();
  __restore((uint64_t)kernel_stack_push_context((TrapContext *)app_init_context(
      APP_BASE_ADDRESS, user_stack_get_sp(),
      (TrapContext *)(kernel_stack_get_sp() - sizeof(TrapContext)))));
  panic("Unreachable in batch.c: run_next_app()\n");
}
