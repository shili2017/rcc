// ref: Elfparse by Barthelemy Delemotte (https://github.com/DBarthe/Elfparse)

#include "elfparse.h"

/*
** Get the program type.
*/
uint32_t elf_program_get_type(t_elf *elf, t_elf_program *program) {
  uint32_t type;

  if (ELF_IS_32(elf))
    type = (uint32_t)program->header->_32.p_type;
  else
    type = (uint32_t)program->header->_64.p_type;
  return (type);
}

/*
** Get the program offset (from the begin of the file)
*/
size_t elf_program_get_offset(t_elf *elf, t_elf_program *program) {
  size_t offset;

  if (ELF_IS_32(elf))
    offset = (size_t)program->header->_32.p_offset;
  else
    offset = (size_t)program->header->_64.p_offset;
  return (offset);
}

/*
** Get the flags of the program
*/
uint64_t elf_program_get_flags(t_elf *elf, t_elf_program *program) {
  uint64_t flags;

  if (ELF_IS_32(elf))
    flags = (uint64_t)program->header->_32.p_flags;
  else
    flags = (uint64_t)program->header->_64.p_flags;
  return (flags);
}

/*
** Get the virtual address of the program
*/
uint64_t elf_program_get_vaddr(t_elf *elf, t_elf_program *program) {
  uint64_t addr;

  if (ELF_IS_32(elf))
    addr = (uint64_t)program->header->_32.p_vaddr;
  else
    addr = (uint64_t)program->header->_64.p_vaddr;
  return (addr);
}

/*
** Get the memory size of the program
*/
uint64_t elf_program_get_memsz(t_elf *elf, t_elf_program *program) {
  uint64_t addr;

  if (ELF_IS_32(elf))
    addr = (uint64_t)program->header->_32.p_memsz;
  else
    addr = (uint64_t)program->header->_64.p_memsz;
  return (addr);
}

/*
** Get the file size of the program
*/
uint64_t elf_program_get_filesz(t_elf *elf, t_elf_program *program) {
  uint64_t addr;

  if (ELF_IS_32(elf))
    addr = (uint64_t)program->header->_32.p_filesz;
  else
    addr = (uint64_t)program->header->_64.p_filesz;
  return (addr);
}
