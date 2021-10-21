/*
** elf_load_sections.c for elfparse in
* /home/common/Dev/tek2/Unix/PSU_2013_nmobjdump/elfparse/sources
**
** Made by Barthelemy Delemotte
** Login   <delemo_b@epitech.net>
**
** Started on Tue Mar  4 16:27:21 2014 Barthelemy Delemotte
** Last update Sun Mar 16 18:00:33 2014 Barthelemy Delemotte
*/

#include "elfparse.h"

/*
** traverse the section header table and load each section
*/
static int elf_load_sections_loop(t_elf *elf, size_t shoff, size_t shnum,
                                  size_t shentsize) {
  size_t i;
  size_t content_offset;
  uint32_t type;

  i = 0;
  while (i < shnum) {
    if (i >= MAX_SECTION_NUM) {
      return (-1);
    }
    elf->sections[i].header =
        (union u_elf_sh *)(elf->raw_data + shoff + i * shentsize);
    type = elf_section_get_type(elf, &elf->sections[i]);
    if (type != SHT_NULL && type != SHT_NOBITS) {
      content_offset = elf_section_get_offset(elf, &elf->sections[i]);
      if (content_offset + elf_section_get_size(elf, &elf->sections[i]) >
          elf->size) {
        return (-1);
      }
      elf->sections[i].content = elf->raw_data + content_offset;
    }
    i++;
  }
  return (0);
}

/*
** Load sections. Create a table of t_elf_section in elf->sections.
** This table is null if there is no section header table.
*/
int elf_load_sections(t_elf *elf) {
  size_t shoff;
  size_t shnum;
  size_t shentsize;

  if ((shoff = elf_header_get_shoff(elf)) > 0) {
    shnum = elf_header_get_shnum(elf);
    shentsize = elf_header_get_shentsize(elf);
    if (shoff + shnum * shentsize > elf->size) {
      return (-1);
    }
    if (elf_load_sections_loop(elf, shoff, shnum, shentsize) == -1) {
      return (-1);
    }
  } else
    memset(elf->sections, 0, MAX_SECTION_NUM * sizeof(t_elf_section));
  return (0);
}
