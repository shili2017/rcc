#include <stdint.h>

#include "config.h"
#include "log.h"
#include "mm.h"
#include "sbi.h"
#include "task.h"

static void print_byte_buffer(uint64_t token, char *buf, uint64_t len) {
  PageTable page_table;
  page_table_from_token(&page_table, token);
  uint64_t start = (uint64_t)buf;
  uint64_t end = start + len;

  VirtAddr start_va, end_va;
  VirtPageNum vpn;
  PhysPageNum ppn;
  uint8_t *bytes_array;

  while (start < end) {
    start_va = (VirtAddr)start;
    vpn = addr_floor(start_va);
    ppn = pte_ppn(*page_table_translate(&page_table, vpn));
    vpn++;
    end_va = (VirtAddr)get_addr_from_page_num(vpn);
    if ((VirtAddr)end < end_va) {
      end_va = (VirtAddr)end;
    }
    bytes_array = ppn_get_bytes_array(ppn);
    if (addr_page_offset(end_va) == 0) {
      for (uint64_t i = addr_page_offset(start_va); i < PAGE_SIZE; i++) {
        console_putchar(bytes_array[i]);
      }
    } else {
      for (uint64_t i = addr_page_offset(start_va);
           i < addr_page_offset(end_va); i++) {
        console_putchar(bytes_array[i]);
      }
    }
    start = (uint64_t)end_va;
  }
}

int64_t sys_write(uint64_t fd, char *buf, uint64_t len) {
  switch (fd) {
  case FD_STDOUT:
    print_byte_buffer(task_current_user_token(), buf, len);
    return (int64_t)len;
  default:
    error("Unsupported fd %lld in sys_write!\n", fd);
    break;
  }
  return -1;
}
