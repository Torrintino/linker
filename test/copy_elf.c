#include "../elf_editor.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if(argc != 3) {
    printf("Invalid argument count\n");
    return 0;
  }
  
  Elf64_Efile* e = Elf64_read_file(argv[1]);
  if(e == NULL) {
    printf("This is not an ELF file\n");
    return 0;
  }
  if(Elf64_write_file(argv[2], e) == -1) {
    printf("Failure during write\n");
    Elf64_destroy_file(e);
    return 1;
  }
  Elf64_destroy_file(e);
  return 0;
}
