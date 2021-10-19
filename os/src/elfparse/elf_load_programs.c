// ref: Elfparse by Barthelemy Delemotte (https://github.com/DBarthe/Elfparse)

#include "elfparse.h"

/*
** traverse the program header table and load each segment
*/
static int elf_load_programs_loop(t_elf *elf, size_t phoff, size_t phnum,
                                  size_t phentsize) {
  size_t i;
  size_t content_offset;
  uint32_t type;

  i = 0;
  while (i < phnum) {
    if (i >= MAX_PROGRAM_NUM) {
      return (-1);
    }
    elf->programs[i].header =
        (union u_elf_ph *)(elf->raw_data + phoff + i * phentsize);
    type = elf_program_get_type(elf, &elf->programs[i]);
    if (type != PT_NULL) {
      content_offset = elf_program_get_offset(elf, &elf->programs[i]);
      elf->programs[i].content = elf->raw_data + content_offset;
    }
    i++;
  }
  return (0);
}

/*
** Load programs. Create a table of t_elf_program in elf->programs.
** This table is null if there is no program header table.
*/
int elf_load_programs(t_elf *elf) {
  size_t phoff;
  size_t phnum;
  size_t phentsize;

  if ((phoff = elf_header_get_phoff(elf)) > 0) {
    phnum = elf_header_get_phnum(elf);
    phentsize = elf_header_get_phentsize(elf);
    if (phoff + phnum * phentsize > elf->size) {
      return (-1);
    }
    if (elf_load_programs_loop(elf, phoff, phnum, phentsize) == -1) {
      return (-1);
    }
  } else
    memset(elf->programs, 0, MAX_PROGRAM_NUM * sizeof(t_elf_program));
  return (0);
}
