#ifndef ELF_EDITOR_H
#define ELF_EDITOR_H

#include <elf.h>

typedef struct {
  Elf64_Ehdr* header;
  Elf64_Phdr* phdr;
  Elf64_Shdr* shdr;
  void** section;
} Elf64_Efile;

Elf64_Efile* Elf64_read_file(const char* path);
void Elf64_print_file(Elf64_Efile* e);
int Elf64_write_file(const char* path, Elf64_Efile* e);

/*** Call to free allocated memory ***/
void Elf64_destroy_file(Elf64_Efile* e);

#endif
