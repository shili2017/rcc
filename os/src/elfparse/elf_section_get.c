/*
** elf_section_get.c for elfparse in
* /home/common/Dev/tek2/Unix/PSU_2013_nmobjdump/elfparse/sources
**
** Made by Barthelemy Delemotte
** Login   <delemo_b@epitech.net>
**
** Started on Wed Mar  5 11:57:59 2014 Barthelemy Delemotte
** Last update Tue Mar 11 12:20:14 2014 Barthelemy Delemotte
*/

#include "elfparse.h"

/*
** Get the section type.
*/
uint32_t elf_section_get_type(t_elf *elf, t_elf_section *section) {
  uint32_t type;

  if (ELF_IS_32(elf))
    type = (uint32_t)section->header->_32.sh_type;
  else
    type = (uint32_t)section->header->_64.sh_type;
  return (type);
}

/*
** Get the section offset (from the begin of the file)
*/
size_t elf_section_get_offset(t_elf *elf, t_elf_section *section) {
  size_t offset;

  if (ELF_IS_32(elf))
    offset = (size_t)section->header->_32.sh_offset;
  else
    offset = (size_t)section->header->_64.sh_offset;
  return (offset);
}

/*
** Get the size of the section
*/
size_t elf_section_get_size(t_elf *elf, t_elf_section *section) {
  size_t size;

  if (ELF_IS_32(elf))
    size = (size_t)section->header->_32.sh_size;
  else
    size = (size_t)section->header->_64.sh_size;
  return (size);
}
