# Linker

This respository contains solutions for the project exercises of the book
"Linkers and Loaders", by John L. Levine.

The target platform is x86-64.

## ELF editor

The ELF editor files contain functions to work with ELF files.
The Elf64_read_file function reads the file and initializes the data structures
defined in elf.h. It defines its own struct for the ELF file itself:

```
typedef struct {
  Elf64_Ehdr* header;
  Elf64_Phdr* phdr;
  Elf64_Shdr* shdr;
  void** section;
  void** segment;
} Elf64_Efile;
```

The sections and segments are treated as raw data and need to be casted by
the user. The user can manipulate the data as needed and then write it back
to a specified file, by calling Elf64_write_file.

The Elf64_print_file function prints the headers and dumps the section and
segment contents in hex format.

With Elf64_create_file and empty file struct can be created.

More convenience functions are planned for the future, such as symbol and
string table manipulation.

For more information about the ELF format read `man elf`.
