CFLAGS = -Wall -Wextra -g
SRCS   = main.c sysenv.c cell.c compound.c common.c

all: dsy-sysenv dsy-exec-cell dsy-dump-cell dsy-edit-cell samples

dsy-sysenv: $(SRCS)
	gcc $(CFLAGS) -o $@ $+

dsy-exec-cell: dsy-exec-cell.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

dsy-dump-cell: dsy-dump-cell.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

dsy-edit-cell: dsy-edit-cell.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

samples:
	make -C $@

clean:
	rm -f *~ tools/*~ dsy-sysenv dsy-exec-cell dsy-dump-cell dsy-edit-cell
	make -C samples clean

.PHONY: samples clean
