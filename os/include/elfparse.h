/*
** elfparse.h for elfparse in
* /home/bade/Dev/tek2/Unix/PSU_2013_nmobjdump/elfparse/includes
**
** Made by Barthelemy Delemotte
** Login   <delemo_b@epitech.net>
**
** Started on Mon Mar  3 17:38:40 2014 Barthelemy Delemotte
** Last update Sun Mar 16 17:52:17 2014 Barthelemy Delemotte
*/

#ifndef _ELFPARSE_H_
#define _ELFPARSE_H_

#include <stdint.h>

#include "elf.h"
#include "string.h"

#define ELF_IS_32(e) ((e)->class == ELFCLASS32)
#define ELF_IS_64(e) ((e)->class == ELFCLASS64)

typedef union {
  Elf32_Ehdr _32;
  Elf64_Ehdr _64;
} t_elf_header;

typedef struct {
  union u_elf_sh {
    Elf32_Shdr _32;
    Elf64_Shdr _64;
  } * header;
  uint8_t *content;
} t_elf_section;

typedef struct {
  union u_elf_ph {
    Elf32_Phdr _32;
    Elf64_Phdr _64;
  } * header;
  uint8_t *content;
} t_elf_program;

#define MAX_SECTION_NUM 64
#define MAX_PROGRAM_NUM 64

typedef struct {
  size_t size;
  uint8_t *raw_data;
  uint8_t *ident;
  uint8_t class;
  t_elf_header *header;
  t_elf_section sections[MAX_SECTION_NUM];
  t_elf_program programs[MAX_PROGRAM_NUM];
} t_elf;

int elf_load(uint8_t *elf_data, size_t elf_size, t_elf *elf);
int elf_load_header(t_elf *elf);
int elf_load_sections(t_elf *elf);
int elf_load_programs(t_elf *elf);

size_t elf_header_get_entry(t_elf *elf);
size_t elf_header_get_shoff(t_elf *elf);
size_t elf_header_get_shentsize(t_elf *elf);
size_t elf_header_get_shnum(t_elf *elf);
size_t elf_header_get_phoff(t_elf *elf);
size_t elf_header_get_phentsize(t_elf *elf);
size_t elf_header_get_phnum(t_elf *elf);

uint32_t elf_section_get_type(t_elf *elf, t_elf_section *section);
size_t elf_section_get_offset(t_elf *elf, t_elf_section *section);
size_t elf_section_get_size(t_elf *elf, t_elf_section *section);

uint32_t elf_program_get_type(t_elf *elf, t_elf_program *program);
size_t elf_program_get_offset(t_elf *elf, t_elf_program *program);
uint64_t elf_program_get_flags(t_elf *elf, t_elf_program *program);
uint64_t elf_program_get_vaddr(t_elf *elf, t_elf_program *program);
uint64_t elf_program_get_memsz(t_elf *elf, t_elf_program *program);
uint64_t elf_program_get_filesz(t_elf *elf, t_elf_program *program);

#endif /* _ELFPARSE_H_ */
