
.PHONY: clean

print_elf: print_elf.c elf_editor.c elf_editor.h
	gcc -g -o print_elf elf_editor.c print_elf.c

clean:
	rm print_elf
