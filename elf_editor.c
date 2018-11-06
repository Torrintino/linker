#include "elf_editor.h"

#include <elf.h>
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

Elf64_Shdr* Elf64_read_section_headers(FILE* f, Elf64_Off offset, uint16_t num) {
  Elf64_Shdr* shdr = malloc(sizeof(Elf64_Shdr) * num);
  if(shdr == NULL)
    return NULL;

  if(fseek(f, offset, SEEK_SET)) {
    free(shdr);
    return NULL;
  }
  if(fread(shdr, sizeof(Elf64_Shdr), num, f) != num) {
    free(shdr);
    return NULL;
  }

  return shdr;
}

void** Elf64_read_sections(FILE* f, Elf64_Shdr* shdr , uint16_t num) {
  void** section = malloc(sizeof(void*) * num);
  if(section == NULL)
    return NULL;

  uint16_t i;
  for(i = 0; i < num; i++) {
    if(shdr[i].sh_size == 0) {
      section[i] = NULL;
    } else {
      section[i] = malloc(shdr[i].sh_size);
      if(section[i] == NULL) goto ALLOCATION_ERROR;
      if(fseek(f, shdr[i].sh_offset, SEEK_SET)) goto FILE_ERROR;
      if(fread(section[i], shdr[i].sh_size, 1, f) != 1) goto FILE_ERROR;
    }
  }

  return section;

 FILE_ERROR:
  free(section[i]);
 ALLOCATION_ERROR:
  for(uint16_t j=i-1; j>=0; j--)
    free(section[j]);
  return NULL;
}

Elf64_Efile* Elf64_read_file(const char* path) {
  Elf64_Efile* e = malloc(sizeof(Elf64_Efile));
  if(e == NULL) return NULL;

  FILE* f = fopen(path, "r");
  if(f == NULL) goto FILE_ERROR;
  
  e->header = Elf64_read_header(f);
  if(e->header == NULL) goto HEADER_ERROR;
  
  e->shdr = Elf64_read_section_headers(f, e->header->e_shoff, e->header->e_shnum);
  if(e->shdr == NULL) goto SECTION_HEADER_ERROR;
  
  e->section = Elf64_read_sections(f, e->shdr, e->header->e_shnum);
  if(e->section == NULL) goto SECTION_ERROR;
  
  fclose(f);
  return e;

 SECTION_ERROR:
  free(e->shdr);
 SECTION_HEADER_ERROR:
  free(e->header);
 HEADER_ERROR:
  fclose(f);
 FILE_ERROR:
  free(e);
  return NULL;
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


const char* shdr_type_dict(int id) {
  switch(id) {
  case SHT_NULL:
    return "NULL";
  case SHT_PROGBITS:
    return "PROGBITS";
  case SHT_SYMTAB:
    return "SYMTAB";
  case SHT_STRTAB:
    return "STRTAB";
  case SHT_RELA:
    return "RELA";
  case SHT_HASH:
    return "HASH";
  case SHT_DYNAMIC:
    return "DYNAMIC";
  case SHT_NOTE:
    return "NOTE";
  case SHT_NOBITS:
    return "NOBITS";
  case SHT_REL:
    return "REL";
  case SHT_SHLIB:
    return "SHLIB";
  case SHT_DYNSYM:
    return "DYNSYM";
  case SHT_LOPROC:
    return "LOPROC";
  case SHT_HIPROC:
    return "HIPROC";
  case SHT_LOUSER:
    return "LOUSER";
  case SHT_HIUSER:
    return "HIUSER";
    
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
  printf("Program entry point: %#" PRIx64 "\n", h->e_entry);
  printf("Program header table offset: %#" PRIx64 "\n", h->e_phoff);
  printf("Section header table offset: %#" PRIx64 "\n", h->e_shoff);
  printf("Flags: 0x%08" PRIx32 "\n", h->e_flags);
  printf("ELF header size: %" PRIu16 "\n", h->e_ehsize);
  printf("Program header size: %" PRIu16 "\n", h->e_phentsize);
  printf("Number of program header table entries: %" PRIu16 "\n", h->e_phnum);
  printf("Section header size: %" PRIu16 "\n", h->e_shentsize);
  printf("Number of section header table entries: %" PRIu16 "\n", h->e_shnum);
  printf("String table index: %" PRIu16 "\n", h->e_shstrndx);
  printf("\n");
}

void Elf64_print_section_headers(Elf64_Shdr* s, uint16_t num,
				 char* string_table) {
  printf("-------------- Section Headers -------------\n\n");
  for(uint16_t i=0; i<num; i++) {
    printf("Name: %s\n", &string_table[s[i].sh_name]);
    printf("Type: %s\n", shdr_type_dict(s[i].sh_type));

    uint64_t flags = s[i].sh_flags;
    int first = 1;
    printf("Flags: %#016" PRIx64, flags);
    if(flags) {
      printf(" (");
      if(flags & SHF_WRITE) {
	printf("SHF_WRITE");
	first = 0;
      }
      if(flags & SHF_ALLOC) {
	if(!first)
	  printf("|");
	printf("SHF_ALLOC");
	first = 0;
      }
      if(flags & SHF_EXECINSTR) {
	if(!first)
	  printf("|");
	printf("SHF_EXECINSTR");
	first = 0;
      }
      if(flags & SHF_MASKPROC) {
	if(!first)
	  printf("|");
	printf("SHF_MASKPROC");
	first = 0;
      }
      printf(")");
    }
    printf("\n");
      
    printf("Address: %#" PRIx64 "\n", s[i].sh_addr);
    printf("Offset: %#" PRIx64 "\n", s[i].sh_offset);
    printf("Size: %" PRIu64 "\n", s[i].sh_size);
    printf("Link: %" PRIu32 "\n", s[i].sh_link);
    printf("Info: %" PRIu32 "\n", s[i].sh_info);
    printf("Address alignment: %" PRIu64 "\n", s[i].sh_addralign);
    printf("Entry size: %" PRIu64 "\n", s[i].sh_entsize);
    printf("\n");
  }
}

void Elf64_print_file(Elf64_Efile* e) {
  printf("---------------- Elf64 file ----------------\n\n");
  Elf64_print_header(e->header);
  Elf64_print_section_headers(e->shdr, e->header->e_shnum,
			      (char*) e->section[e->header->e_shstrndx]);
}

void Elf64_destroy_file(Elf64_Efile* e) {
  for(uint16_t i = 0; i < e->header->e_shnum; i++)
    free(e->section[i]);
  free(e->section);
  free(e->header);
  free(e->shdr);
  free(e);
}
