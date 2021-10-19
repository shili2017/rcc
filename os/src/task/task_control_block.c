#include "config.h"
#include "task.h"

const TaskContext **get_task_cx_ptr2(TaskControlBlock *s) {
  return (const TaskContext **)(&(s->task_cx_ptr));
}

TrapContext *get_trap_cx(TaskControlBlock *s) {
  return (TrapContext *)get_addr_from_page_num(s->trap_cx_ppn);
}

uint64_t get_user_token(TaskControlBlock *s) {
  return memory_set_token(&s->memory_set);
}

void task_control_block_new(uint8_t *elf_data, size_t elf_size, uint64_t app_id,
                            TaskControlBlock *s) {
  // memory_set with elf program headers/trampoline/trap context/user stack
  uint64_t user_sp;
  uint64_t entry_point;
  memory_set_from_elf(&s->memory_set, elf_data, elf_size, &user_sp,
                      &entry_point);
  PhysPageNum trap_cx_ppn = pte_ppn(*memory_set_translate(
      &s->memory_set,
      (VirtPageNum)get_page_num_from_addr((VirtAddr)TRAP_CONTEXT)));

  // map a kernel-stack in kernel space
  uint64_t kernel_stack_bottom = kernel_stack_position_bottom(app_id);
  uint64_t kernel_stack_top = kernel_stack_position_top(app_id);
  kernel_space_insert_framed_area((VirtAddr)kernel_stack_bottom,
                                  (VirtAddr)kernel_stack_top,
                                  MAP_PERM_R | MAP_PERM_W);

  TaskContext *task_cx_ptr =
      (TaskContext *)(kernel_stack_top - sizeof(TaskContext));
  task_context_goto_trap_return(task_cx_ptr);

  s->task_cx_ptr = task_cx_ptr;
  s->task_status = TASK_STATUS_READY;
  s->trap_cx_ppn = trap_cx_ppn;
  s->base_size = user_sp;

  // prepare TrapContext in user space
  TrapContext *trap_cx = get_trap_cx(s);
  app_init_context(entry_point, user_sp, kernel_space_token(), kernel_stack_top,
                   (uint64_t)trap_handler, trap_cx);

  s->priority = DEFAULT_PRIORITY;
  s->stride = 0;
}
