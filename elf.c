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

const char* ofc_dict(int id) {
  switch(id) {
  case ELFCLASS32:
    return "32-bit objects";
  case ELFCLASS64:
    return "64-bit objects";
  default:
    return "Unknown";
  }
}

const char* data2_dict(int id) {
  switch(id) {
  case ELFDATA2LSB:
    return "little-endian";
  case ELFDATA2MSB:
    return "big-endian";
  default:
    return "Unknown"; 
  }
}

const char* osabi_dict(int id) {
  switch(id) {
  case ELFOSABI_SYSV:
    return "System V ABI";
  case ELFOSABI_HPUX:
    return "HP-UX operating system";
  case ELFOSABI_STANDALONE:
    return "Standalone (embedded) application";
  default:
    return "Unknown";
  }
};

const char* oft_dict(int id) {
  switch(id) {
  case ET_NONE:
    return "No file type";
  case ET_REL:
    return "Relocatable file";
  case ET_EXEC:
    return "Executable file";
  case ET_DYN:
    return "Shared object file";
  case ET_CORE:
    return "Core file";
  case ET_LOOS:
  case ET_HIOS:
    return "Environment-specific use";
  case ET_LOPROC:
  case ET_HIPROC:
    return "Processor-specific use";
  default:
    return "Unknown";
    }
}

void Elf64_print_header(Elf64_Ehdr* h) {
  printf("------------------ Header ------------------\n\n");
  printf("File class: %d (%s)\n", h->e_ident[4],
	 ofc_dict(h->e_ident[4]));
  printf("Data encoding: %d (%s)\n", h->e_ident[5],
	 data2_dict(h->e_ident[5]));
  printf("File version: %d\n", h->e_ident[6]);
  printf("OS/ABI identification: %d (%s)\n", h->e_ident[7],
	 osabi_dict(h->e_ident[7]));
  printf("ABI version: %d\n", h->e_ident[8]);
  printf("Object file type: %" PRIu16 " (%s)\n",
	 h->e_type, oft_dict(h->e_type));
  printf("Target architecture: %" PRIu16 "\n", h->e_machine);
  printf("Version: %" PRIu32 "\n", h->e_version);
  printf("Program entry point: %#016" PRIx64 "\n", h->e_entry);
  printf("Program header table offset: %#016" PRIx64 "\n", h->e_phoff);
  printf("Section header table offset: %#016" PRIx64 "\n", h->e_shoff);
  printf("Flags: 0x%08" PRIx32 "\n", h->e_flags);
  printf("ELF header size: %" PRIu16 "\n", h->e_ehsize);
  printf("Program header size: %" PRIu16 "\n", h->e_phentsize);
  printf("Number of program header table entries: %" PRIu16 "\n", h->e_phnum);
  printf("Section header size: %" PRIu16 "\n", h->e_shentsize);
  printf("Number of section header table entries: %" PRIu16 "\n", h->e_shnum);
  printf("String table index: %" PRIu16 "\n", h->e_shstrndx);
}

void Elf64_print_file(Elf64_Efile* e) {
  printf("---------------- Elf64 file ----------------\n\n");
  Elf64_print_header(e->header);
}

void Elf64_destroy_file(Elf64_Efile* e) {
  free(e->header);
  free(e);
}
