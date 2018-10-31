
.PHONY: clean

print_elf: print_elf.c elf.c elf.h
	gcc -g -o print_elf elf.c print_elf.c

clean:
	rm print_elf
