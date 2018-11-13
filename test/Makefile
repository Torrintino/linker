
.PHONY: clean

print_elf: print_elf.c ../elf_editor.c ../elf_editor.h
	gcc -g -o print_elf ../elf_editor.c print_elf.c

copy_elf: copy_elf.c ../elf_editor.c ../elf_editor.h
	gcc -g -o copy_elf ../elf_editor.c copy_elf.c

create_elf: create_elf.c ../elf_editor.c ../elf_editor.h
	gcc -g -o create_elf ../elf_editor.c create_elf.c

clean:
	rm print_elf copy_elf create_elf log.txt

test: print_elf
	fw/valgrind_test.py ./print_elf
	fw/valgrind_test.py ./print_elf nonexistingfile
	fw/valgrind_test.py ./print_elf empty_file
	fw/valgrind_test.py ./print_elf short_elf
	fw/valgrind_test.py ./print_elf false_sh_offset
	fw/valgrind_test.py ./print_elf broken_sh
	./copy_elf copy_elf copy.o && readelf -a copy.o > copy && readelf -a copy_elf > orig && diff copy orig && rm copy orig copy.o

