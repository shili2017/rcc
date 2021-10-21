/*
** elf_header_get_sh.c for elfparse in
* /home/common/Dev/tek2/Unix/PSU_2013_nmobjdump/elfparse/sources
**
** Made by Barthelemy Delemotte
** Login   <delemo_b@epitech.net>
**
** Started on Tue Mar  4 15:52:12 2014 Barthelemy Delemotte
** Last update Mon Mar 10 21:36:21 2014 Barthelemy Delemotte
*/

#include "elfparse.h"

/*
** Get the section header table offset
*/
size_t elf_header_get_shoff(t_elf *elf) {
  size_t shoff;

  if (ELF_IS_32(elf))
    shoff = (size_t)elf->header->_32.e_shoff;
  else
    shoff = (size_t)elf->header->_64.e_shoff;
  return (shoff);
}

/*
** Get the size of one section header
*/
size_t elf_header_get_shentsize(t_elf *elf) {
  size_t shentsize;

  if (ELF_IS_32(elf))
    shentsize = (size_t)elf->header->_32.e_shentsize;
  else
    shentsize = (size_t)elf->header->_64.e_shentsize;
  return (shentsize);
}

/*
** Get the number of entry in the section header table
*/
size_t elf_header_get_shnum(t_elf *elf) {
  size_t shum;

  if (ELF_IS_32(elf))
    shum = (size_t)elf->header->_32.e_shnum;
  else
    shum = (size_t)elf->header->_64.e_shnum;
  return (shum);
}
