#ifndef ELF_H
#define ELF_H

#include <stdint.h>

typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Off;
typedef int32_t  Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_SxWord;
typedef uint64_t Elf64_XWord;

#define EI_NIDENT 16

typedef struct Elf64_Ehdr {
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;
  Elf64_Off e_phoff;
  Elf64_Off e_shoff;
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

/*** Type ***/
enum { ET_NONE,
       ET_REL,
       ET_EXEC,
       ET_DYN,
       ET_CORE,
       ET_LOOS = 0xFE00,
       ET_HIOS = 0xFEFF,
       ET_LOPROC = 0xFF00,
       ET_HIPROC = 0xFFFF };

/*** e_version ***/
enum { EV_NONE, EV_CURRENT };

/*** Object File Classes ***/
enum { ELFCLASS32 = 1, ELFCLASS64 };

/*** Data Encodings ***/
enum { ELFDATA2LSB = 1, ELFDATA2MSB };

/*** ABI Identifier ***/
enum { ELFOSABI_SYSV,
       ELFOSABI_HPUX,
       ELFOSABI_STANDALONE = 255
};

typedef struct Elf64_Efile {
  Elf64_Ehdr* header;
} Elf64_Efile;

Elf64_Efile* Elf64_read_file(const char* path);
void Elf64_print_file(Elf64_Efile* e);

/*** Call to free allocated memory ***/
void Elf64_destroy_file(Elf64_Efile* e);

#endif
