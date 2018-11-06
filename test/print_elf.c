#include "../elf_editor.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if(argc != 2) {
    printf("Invalid argument count\n");
    return 0;
  }
  
  Elf64_Efile* e = Elf64_read_file(argv[1]);
  if(e == NULL) {
    printf("This is not an ELF file\n");
    return 0;
  }
  Elf64_print_file(e);
  Elf64_destroy_file(e);
}
