#include "elf_editor.h"

#include <elf.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

/*** Init ***/

Elf64_Efile* Elf64_create_file() {
  Elf64_Efile* e = malloc(sizeof(Elf64_Efile));
  if(e == NULL)
    return NULL;

  e->header = malloc(sizeof(Elf64_Ehdr));
  if(e->header == NULL) {
    free(e);
    return NULL;
  }
  e->header->e_ident[0] = 0x7f;
  e->header->e_ident[1] = 'E';
  e->header->e_ident[2] = 'L';
  e->header->e_ident[3] = 'F';
  e->header->e_ident[EI_CLASS] = ELFCLASS64;
  e->header->e_ident[EI_DATA] = ELFDATA2LSB;
  e->header->e_ident[EI_VERSION] = EV_CURRENT;
  e->header->e_ident[EI_OSABI] = ELFOSABI_SYSV;
  e->header->e_ident[EI_ABIVERSION] = 0;
  for(int i=EI_PAD; i<EI_NIDENT; i++)
    e->header->e_ident[i] = 0;
  e->header->e_type = ET_NONE;
  e->header->e_machine = EM_X86_64;
  e->header->e_version = EV_CURRENT;
  e->header->e_entry = 0;
  e->header->e_phoff = 0;
  e->header->e_shoff = 0;
  e->header->e_flags = 0;
  e->header->e_ehsize = sizeof(Elf64_Ehdr);
  e->header->e_phentsize = sizeof(Elf64_Phdr);
  e->header->e_phnum = 0;
  e->header->e_shentsize = sizeof(Elf64_Shdr);
  e->header->e_shnum = 0;
  e->header->e_shstrndx = 0;
  
  e->phdr = NULL;
  e->shdr = NULL;
  e->section = NULL;
  e->segment = NULL;

  return e;
}

/*** Read ***/

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

Elf64_Shdr* Elf64_read_section_headers(FILE* f, Elf64_Off offset,
				       uint16_t num) {
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

Elf64_Phdr* Elf64_read_program_headers(FILE* f, Elf64_Off offset,
				       uint16_t num) {
  Elf64_Phdr* phdr = malloc(sizeof(Elf64_Phdr) * num);
  if(phdr == NULL)
    return NULL;

  if(fseek(f, offset, SEEK_SET)) {
    free(phdr);
    return NULL;
  }
  if(fread(phdr, sizeof(Elf64_Phdr), num, f) != num) {
    free(phdr);
    return NULL;
  }

  return phdr;
}

void** Elf64_read_segments(FILE* f, Elf64_Phdr* phdr, uint16_t num) {
  void** segment = malloc(sizeof(void*) * num);
  if(segment == NULL)
    return NULL;

  uint16_t i;
  for(i = 0; i < num; i++) {
    if(phdr[i].p_filesz == 0) {
      segment[i] = NULL;
    } else {
      segment[i] = malloc(phdr[i].p_filesz);
      if(segment[i] == NULL) goto ALLOCATION_ERROR;
      if(fseek(f, phdr[i].p_offset, SEEK_SET)) goto FILE_ERROR;
      if(fread(segment[i], phdr[i].p_filesz, 1, f) != 1) goto FILE_ERROR;
    }
  }

  return segment;

 FILE_ERROR:
  free(segment[i]);
 ALLOCATION_ERROR:
  for(uint16_t j=i-1; j>=0; j--)
    free(segment[j]);
  return NULL;
}

Elf64_Efile* Elf64_read_file(const char* path) {
  Elf64_Efile* e = malloc(sizeof(Elf64_Efile));
  if(e == NULL) return NULL;

  FILE* f = fopen(path, "r");
  if(f == NULL) goto FILE_ERROR;
  
  e->header = Elf64_read_header(f);
  if(e->header == NULL) goto HEADER_ERROR;

  if(e->header->e_shnum == 0) {
    e->shdr = NULL;
    e->section = NULL;
  } else {
    e->shdr = Elf64_read_section_headers(f, e->header->e_shoff,
					 e->header->e_shnum);
    if(e->shdr == NULL) goto SECTION_HEADER_ERROR;
  
    e->section = Elf64_read_sections(f, e->shdr, e->header->e_shnum);
    if(e->section == NULL) goto SECTION_ERROR;
  }

  if(e->header->e_phnum == 0) {
    e->phdr = NULL;
    e->segment = NULL;
  } else {
    e->phdr = Elf64_read_program_headers(f, e->header->e_phoff,
					 e->header->e_phnum);
    if(e->phdr == NULL) goto PROGRAM_HEADER_ERROR;

    e->segment = Elf64_read_segments(f, e->phdr, e->header->e_phnum);
    if(e->segment == NULL) goto SEGMENT_ERROR;
  }
  
  fclose(f);
  return e;

 SEGMENT_ERROR:
  free(e->phdr);
 PROGRAM_HEADER_ERROR:
  free(e->section);
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

/*** Dictionaries ***/

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

const char* phdr_type_dict(int id) {
  switch(id) {
  case PT_NULL: return "NULL";
  case PT_LOAD: return "LOAD";
  case PT_DYNAMIC: return "DYNAMIC";
  case PT_INTERP: return "INTERP";
  case PT_NOTE: return "NOTE";
  case PT_SHLIB: return "SHLIB";
  case PT_PHDR: return "PHDR";
  case PT_LOPROC: return "LOPROC";
  case PT_HIPROC: return "HIPROC";
  case PT_GNU_STACK: return "GNU_STACK";
  default: return "Unknown";
  }
}

/*** Print ***/

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

void Elf64_print_section_header(Elf64_Shdr* s, char* string_table) {
  printf("---- Header ----\n");
  printf("Name: %s\n", &string_table[s->sh_name]);
  printf("Type: %s\n", shdr_type_dict(s->sh_type));

  uint64_t flags = s->sh_flags;
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
      
  printf("Address: %#" PRIx64 "\n", s->sh_addr);
  printf("Offset: %#" PRIx64 "\n", s->sh_offset);
  printf("Size: %" PRIu64 "\n", s->sh_size);
  printf("Link: %" PRIu32 "\n", s->sh_link);
  printf("Info: %" PRIu32 "\n", s->sh_info);
  printf("Address alignment: %" PRIu64 "\n", s->sh_addralign);
  printf("Entry size: %" PRIu64 "\n", s->sh_entsize);
  printf("\n");
}

#define HEX_WIDTH 16
void hex_dump(void* s, uint16_t size) {
  for(uint16_t j=0; j<size; j += HEX_WIDTH) {
    printf("%02" PRIx8, ((unsigned char*) s)[j]);
    for(int k=1; k<HEX_WIDTH; k++) {
      if(j + HEX_WIDTH <= size ||
	 k < size % HEX_WIDTH) {
	printf(" %02" PRIx8, ((unsigned char*) s)[j + k]);
      }
    }
    printf("\n");
  }
}

void Elf64_print_sections(Elf64_Shdr* shdr, void** section, uint16_t num,
			  char* string_table) {
  if(num == 0) {
    printf("This file doesn't contain sections.\n");
    return;
  }
  printf("----------------- Sections -----------------\n\n");
  for(uint16_t i=0; i<num; i++) {
    Elf64_print_section_header(&shdr[i], string_table);

    printf("---- Content ----\n");
    hex_dump(section[i], shdr[i].sh_size);
    printf("\n");
  }
}

void Elf64_print_program_header(Elf64_Phdr* p) {
  printf("---- Header ----\n");
  printf("Type: %s\n", phdr_type_dict(p->p_type));
  printf("Flags: %#08" PRIx32 "\n", p->p_flags);
  printf("Offset: %#" PRIx64 "\n", p->p_offset);
  printf("Virtual address: %#" PRIx64 "\n", p->p_vaddr);
  printf("Physical address: %#" PRIx64 "\n", p->p_paddr);
  printf("File size: %" PRIu64 "\n", p->p_filesz);
  printf("Memory size: %" PRIu64 "\n", p->p_memsz);
  printf("Alignment: %" PRIu64 "\n\n", p->p_align);
}

void Elf64_print_segments(Elf64_Phdr* phdr, void** segment, uint16_t num) {
  if(num == 0) {
    printf("This file doesn't contain segments.\n");
    return;
  }
  
  printf("----------------- Segments -----------------\n\n");
  for(uint16_t i=0; i<num; i++) {
    Elf64_print_program_header(&phdr[i]);

    printf("---- Content ----\n");
    hex_dump(segment[i], phdr[i].p_filesz);
    printf("\n");
  }
}

void Elf64_print_file(Elf64_Efile* e) {
  printf("---------------- Elf64 file ----------------\n\n");
  Elf64_print_header(e->header);
  Elf64_print_sections(e->shdr, e->section, e->header->e_shnum,
		       (char*) e->section[e->header->e_shstrndx]);
  Elf64_print_segments(e->phdr, e->segment, e->header->e_phnum);
}

/*** Write ***/

int Elf64_write_file(const char* path, Elf64_Efile* e) {
  FILE* f = fopen(path, "w");
  if(f == NULL)
    return -1;
  if(fwrite(e->header, e->header->e_ehsize, 1, f) != 1) goto FILE_ERROR;
  
  if(e->phdr) {
    uint16_t num = e->header->e_phnum;
    if(fseek(f, e->header->e_phoff, SEEK_SET) == -1)
      goto FILE_ERROR;
    if(fwrite(e->phdr, e->header->e_phentsize, num, f) != num)
      goto FILE_ERROR;
    
    for(uint16_t i=0; i<num; i++) {
      if(e->phdr[i].p_filesz > 0) {
	if(fseek(f, e->phdr[i].p_offset, SEEK_SET) == -1)
	  goto FILE_ERROR;
	if(fwrite(e->segment[i], e->phdr[i].p_filesz, 1, f) != 1)
	  goto FILE_ERROR;
      }
    }
  }
  
  if(e->shdr) {
    uint16_t num = e->header->e_shnum;
    if(fseek(f, e->header->e_shoff, SEEK_SET) == -1)
      goto FILE_ERROR;
    if(fwrite(e->shdr, e->header->e_shentsize, num, f) != num)
      goto FILE_ERROR;
    for(uint16_t i=0; i<num; i++) {
      if(e->shdr[i].sh_size > 0) {
	if(fseek(f, e->shdr[i].sh_offset, SEEK_SET) == -1)
	  goto FILE_ERROR;
	if(fwrite(e->section[i], e->shdr[i].sh_size, 1, f) != 1)
	  goto FILE_ERROR;
      }
    }
  }
  
  fclose(f);
  return 0;
  
 FILE_ERROR:
  fclose(f);
  return -1;
}

/*** Destructor ***/

void Elf64_destroy_file(Elf64_Efile* e) {
  for(uint16_t i = 0; i < e->header->e_shnum; i++)
    free(e->section[i]);
  for(uint16_t i = 0; i < e->header->e_phnum; i++)
    free(e->segment[i]);
  free(e->section);
  free(e->segment);
  free(e->header);
  free(e->shdr);
  free(e->phdr);
  free(e);
}
