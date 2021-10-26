#include "config.h"
#include "task.h"

TrapContext *task_control_block_get_trap_cx(TaskControlBlock *s) {
  return (TrapContext *)pn2addr(s->trap_cx_ppn);
}

uint64_t task_control_block_get_user_token(TaskControlBlock *s) {
  return memory_set_token(&s->memory_set);
}

TaskStatus task_control_block_get_status(TaskControlBlock *s) {
  return s->task_status;
}

void task_control_block_new(TaskControlBlock *s, uint8_t *elf_data,
                            size_t elf_size) {
  // memory_set with elf program headers/trampoline/trap context/user stack
  uint64_t user_sp;
  uint64_t entry_point;
  memory_set_from_elf(&s->memory_set, elf_data, elf_size, &user_sp,
                      &entry_point);
  s->trap_cx_ppn = (PhysPageNum)pte_ppn(*memory_set_translate(
      &s->memory_set, (VirtPageNum)addr2pn((VirtAddr)TRAP_CONTEXT)));

  // alloc a pid and a kernel stack in kernel space
  s->pid = pid_alloc();
  kernel_stack_new(&s->kernel_stack, s->pid);
  uint64_t kernel_stack_top = kernel_stack_get_top(&s->kernel_stack);

  // push a task context which goes to trap_return to the top of kernel stack
  s->base_size = user_sp;
  task_context_goto_trap_return(&s->task_cx, kernel_stack_top);
  s->task_status = TASK_STATUS_READY;
  s->parent = NULL;
  vector_new(&s->children, sizeof(TaskControlBlock *));
  s->exit_code = 0;

  // prepare TrapContext in user space
  TrapContext *trap_cx = task_control_block_get_trap_cx(s);
  app_init_context(entry_point, user_sp, kernel_space_token(), kernel_stack_top,
                   (uint64_t)trap_handler, trap_cx);

  s->priority = DEFAULT_PRIORITY;
  s->stride = 0;
}

void task_control_block_free(TaskControlBlock *s) {
  memory_set_free(&s->memory_set);
  vector_free(&s->children);
}

void task_control_block_exec(TaskControlBlock *s, uint8_t *elf_data,
                             size_t elf_size) {
  // memory_set with elf program headers/trampoline/trap context/user stack
  uint64_t user_sp;
  uint64_t entry_point;
  // substitute memory_set
  memory_set_from_elf(&s->memory_set, elf_data, elf_size, &user_sp,
                      &entry_point);

  // update trap_cx ppn
  s->trap_cx_ppn = (PhysPageNum)pte_ppn(*memory_set_translate(
      &s->memory_set, (VirtPageNum)addr2pn((VirtAddr)TRAP_CONTEXT)));

  // initialize trap_cx
  TrapContext *trap_cx = task_control_block_get_trap_cx(s);
  uint64_t kernel_stack_top = kernel_stack_get_top(&s->kernel_stack);
  app_init_context(entry_point, user_sp, kernel_space_token(), kernel_stack_top,
                   (uint64_t)trap_handler, trap_cx);
}

TaskControlBlock *task_control_block_fork(TaskControlBlock *parent) {
  TaskControlBlock *s = (TaskControlBlock *)bd_malloc(sizeof(TaskControlBlock));

  // copy user space (include trap context)
  memory_set_from_existed_user(&s->memory_set, &parent->memory_set);
  s->trap_cx_ppn = (PhysPageNum)pte_ppn(*memory_set_translate(
      &s->memory_set, (VirtPageNum)addr2pn((VirtAddr)TRAP_CONTEXT)));

  // alloc a pid and a kernel stack in kernel space
  s->pid = pid_alloc();
  kernel_stack_new(&s->kernel_stack, s->pid);
  uint64_t kernel_stack_top = kernel_stack_get_top(&s->kernel_stack);

  s->base_size = parent->base_size;
  task_context_goto_trap_return(&s->task_cx, kernel_stack_top);
  s->task_status = TASK_STATUS_READY;
  s->parent = parent;
  vector_new(&s->children, sizeof(TaskControlBlock *));
  s->exit_code = 0;

  s->priority = parent->priority;
  s->stride = parent->stride;

  // add child
  vector_push(&parent->children, s);

  // prepare TrapContext in user space
  TrapContext *trap_cx = task_control_block_get_trap_cx(s);
  trap_cx->kernel_sp = kernel_stack_top;

  return s;
}

uint64_t task_control_block_getpid(TaskControlBlock *s) {
  return (uint64_t)s->pid;
}
