#include "log.h"
#include "task.h"

static PidAllocator PID_ALLOCATOR;

void pid_allocator_init() {
  PID_ALLOCATOR.current = 1;
  vector_new(&PID_ALLOCATOR.recycled, sizeof(PidHandle));
}

void pid_allocator_free() { vector_free(&PID_ALLOCATOR.recycled); }

PidHandle pid_alloc() {
  PidHandle pid;
  if (!vector_empty(&PID_ALLOCATOR.recycled)) {
    pid = *(PidHandle *)vector_back(&PID_ALLOCATOR.recycled);
    vector_pop(&PID_ALLOCATOR.recycled);
  } else {
    pid = PID_ALLOCATOR.current;
    PID_ALLOCATOR.current++;
  }
  return pid;
}

void pid_dealloc(PidHandle pid) {
  bool in_recycled = false;
  PidHandle *x = (PidHandle *)(&PID_ALLOCATOR.recycled.buffer);
  for (uint64_t i = 0; i < PID_ALLOCATOR.recycled.size; i++) {
    if (x[i] == pid) {
      in_recycled = true;
      break;
    }
  }
  if (pid >= PID_ALLOCATOR.current || in_recycled) {
    panic("Pid=%llx has not been allocated!\n", pid);
  }
  vector_push(&PID_ALLOCATOR.recycled, &pid);
}

extern MemorySet KERNEL_SPACE;

void kernel_stack_new(KernelStack *ks, PidHandle pid) {
  uint64_t kernel_stack_bottom = kernel_stack_position_bottom(pid);
  uint64_t kernel_stack_top = kernel_stack_position_top(pid);
  kernel_space_insert_framed_area(kernel_stack_bottom, kernel_stack_top,
                                  MAP_PERM_R | MAP_PERM_W);
  ks->pid = pid;
}

void kernel_stack_free(KernelStack *ks) {
  VirtAddr kernel_stack_bottom_va =
      (VirtAddr)kernel_stack_position_bottom(ks->pid);
  VirtPageNum kernel_stack_bottom_vpn = addr2pn(kernel_stack_bottom_va);
  kernel_space_remove_area_with_start_vpn(kernel_stack_bottom_vpn);
}

uint64_t kernel_stack_get_top(KernelStack *ks) {
  return (uint64_t)kernel_stack_position_top(ks->pid);
}
