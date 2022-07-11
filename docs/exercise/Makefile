CFLAGS = -Wall -Wextra

all: create-elf-header

create-elf-header: create-elf-header.c
	gcc $(CFLAGS) -o $@ $+

clean:
	rm -f *~ create-elf-header

.PHONY: clean
