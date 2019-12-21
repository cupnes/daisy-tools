CFLAGS = -Wall -Wextra
SRCS   = main.c

all: dsy-sysenv

dsy-sysenv: $(SRCS)
	gcc $(CFLAGS) -o $@ $+

clean:
	rm -f *~ dsy-sysenv

.PHONY: clean
