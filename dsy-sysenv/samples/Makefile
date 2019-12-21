CFLAGS = -Wall -Wextra

all: incrementer

incrementer: incrementer.c ../sysenv.c	../cell.c ../compound.c ../common.c
	gcc $(CFLAGS) -o $@ $+

clean:
	rm -f *~ incrementer

.PHONY: clean
