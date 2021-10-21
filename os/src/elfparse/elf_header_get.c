/*
** elf_header_get.c for elfparse in
* /home/common/Dev/tek2/Unix/PSU_2013_nmobjdump/objdump/sources
**
** Made by Barthelemy Delemotte
** Login   <delemo_b@epitech.net>
**
** Started on Mon Mar 10 20:36:56 2014 Barthelemy Delemotte
** Last update Mon Mar 10 20:51:03 2014 Barthelemy Delemotte
*/

#include "elfparse.h"

size_t elf_header_get_entry(t_elf *elf) {
  size_t entry;

  if (ELF_IS_32(elf))
    entry = (size_t)elf->header->_32.e_entry;
  else
    entry = (size_t)elf->header->_64.e_entry;
  return (entry);
}
