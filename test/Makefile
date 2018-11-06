
.PHONY: clean

print_elf: print_elf.c ../elf_editor.c ../elf_editor.h
	gcc -g -o print_elf ../elf_editor.c print_elf.c

clean:
	rm print_elf


test: print_elf
	fw/valgrind_test.py ./print_elf
	fw/valgrind_test.py ./print_elf nonexistingfile
	fw/valgrind_test.py ./print_elf empty_file
	fw/valgrind_test.py ./print_elf short_elf
	fw/valgrind_test.py ./print_elf false_sh_offset
	fw/valgrind_test.py ./print_elf broken_sh

