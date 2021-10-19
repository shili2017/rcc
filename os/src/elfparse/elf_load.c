// ref: Elfparse by Barthelemy Delemotte (https://github.com/DBarthe/Elfparse)

#include "elfparse.h"

/*
** Load the elf file present in the buffer.
** This function is used to parse elf file contained in a static library
** archive.
*/
int elf_load(uint8_t *elf_data, size_t elf_size, t_elf *elf) {
  int ret = 0;
  memset(elf, 0, sizeof(t_elf));
  elf->size = elf_size;
  elf->raw_data = elf_data;

  ret |= elf_load_header(elf);
  ret |= elf_load_sections(elf);
  ret |= elf_load_programs(elf);

  return ret;
}
