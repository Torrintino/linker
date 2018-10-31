#include "elf.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

/*** Reads size from header and allocates necessary storage ***/
Elf64_Ehdr* Elf64_read_header(FILE* f) {
  Elf64_Ehdr* h = malloc(sizeof(Elf64_Ehdr));
  if(h == NULL)
    return NULL;
  
  if(fread(h, sizeof(Elf64_Ehdr), 1, f) == 0) {
    free(h);
    return NULL;
  }

  if(h->e_ident[0] != 127 ||
     h->e_ident[1] != 'E' ||
     h->e_ident[2] != 'L' ||
     h->e_ident[3] != 'F') {
    free(h);
    return NULL;
  }
  
  return h;
}

Elf64_Efile* Elf64_read_file(const char* path) {
  Elf64_Efile* e = malloc(sizeof(Elf64_Efile*));

  FILE* f = fopen(path, "r");
  if(f == NULL)
    return NULL;
  
  e->header = Elf64_read_header(f);
  if(e->header == NULL) {
    free(e);
    fclose(f);
    return NULL;
  }
  
  fclose(f);
  return e;
}

void Elf64_print_header(Elf64_Ehdr* h) {
  printf("Header:\n");
  //printf("%s\n", h->e_ident);
  printf("e_type: %" PRIu16 "\n", h->e_type);
  printf("e_machine: %" PRIu16 "\n", h->e_machine);
  printf("e_version: %" PRIu32 "\n", h->e_version);
  printf("e_entry: %#" PRIx64 "\n", h->e_entry);
  printf("e_phoff: %" PRIu64 "\n", h->e_phoff);
  printf("e_shoff: %" PRIu64 "\n", h->e_shoff);
  printf("e_flags: %#" PRIx32 "\n", h->e_flags);
  printf("e_ehsize: %" PRIu16 "\n", h->e_ehsize);
  printf("e_phentsize: %" PRIu16 "\n", h->e_phentsize);
  printf("e_phnum: %" PRIu16 "\n", h->e_phnum);
  printf("e_shentsize: %" PRIu16 "\n", h->e_shentsize);
  printf("e_shnum: %" PRIu16 "\n", h->e_shnum);
  printf("e_shstrdnx: %" PRIu16 "\n", h->e_shstrndx);
}

void Elf64_print_file(Elf64_Efile* e) {
  printf("Elf64 file:\n\n");
  Elf64_print_header(e->header);
}

void Elf64_destroy_file(Elf64_Efile* e) {
  free(e->header);
  free(e);
}
