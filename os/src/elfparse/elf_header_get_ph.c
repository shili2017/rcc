/*
** elf_header_get_ph.c for elfparse in
* /home/common/Dev/tek2/Unix/PSU_2013_nmobjdump/elfparse/sources
**
** Made by Barthelemy Delemotte
** Login   <delemo_b@epitech.net>
**
** Started on Tue Mar  4 15:52:12 2014 Barthelemy Delemotte
** Last update Tue Mar  4 16:22:04 2014 Barthelemy Delemotte
*/

#include "elfparse.h"

/*
** Get the program header table offset
*/
size_t elf_header_get_phoff(t_elf *elf) {
  size_t phoff;

  if (ELF_IS_32(elf))
    phoff = (size_t)elf->header->_32.e_phoff;
  else
    phoff = (size_t)elf->header->_64.e_phoff;
  return (phoff);
}

/*
** Get the size of one program header
*/
size_t elf_header_get_phentsize(t_elf *elf) {
  size_t phentsize;

  if (ELF_IS_32(elf))
    phentsize = (size_t)elf->header->_32.e_phentsize;
  else
    phentsize = (size_t)elf->header->_64.e_phentsize;
  return (phentsize);
}

/*
** Get the number of entry in the program header table
*/
size_t elf_header_get_phnum(t_elf *elf) {
  size_t phnum;

  if (ELF_IS_32(elf))
    phnum = (size_t)elf->header->_32.e_phnum;
  else
    phnum = (size_t)elf->header->_64.e_phnum;
  return (phnum);
}
