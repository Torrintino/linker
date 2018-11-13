#include "../elf_editor.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if(argc != 2) {
    printf("Invalid argument count\n");
    return 0;
  }
  
  Elf64_Efile* e = Elf64_create_file();
  if(e == NULL) {
    printf("Creation failed\n");
    return 0;
  }
  if(Elf64_write_file(argv[1], e) == -1) {
    printf("Write Failure\n");
    Elf64_destroy_file(e);
    return 1;
  }
  
  Elf64_destroy_file(e);
  return 0;
}
