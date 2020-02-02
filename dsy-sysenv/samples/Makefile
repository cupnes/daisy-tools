TARGETS = create-init-cell create-incr-cell-data create-exit-cell	\
		create-exit_1-cell create-puta-cell-code		\
		create-hello-cell-code
CFLAGS = -Wall -Wextra
# CFLAGS += -static
COMMON_SRCS = ../sysenv.c ../cell.c ../compound.c ../common.c
WORK_DIR ?= $(HOME)/dsy-work

all: $(TARGETS)

create-init-cell: create-init-cell.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-incr-cell-data: create-incr-cell-data.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-exit-cell: create-exit-cell.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-exit_1-cell: create-exit_1-cell.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-puta-cell-code: create-puta-cell-code.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-hello-cell-code: create-hello-cell-code.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

setup: $(TARGETS) create-exit-codes create-syscall-code dsy-name	\
	dsy-eval-init dsy-eval-exit dsy-eval-exit_0 dsy-eval-incr	\
	dsy-eval-puta dsy-eval-puta_0 dsy-eval-hello			\
	cell-fitness-logger dsy-log2hist dsy-show-hists
	mkdir -p $(WORK_DIR)/samples
	cp $+ $(WORK_DIR)/samples/

clean:
	rm -f *~ $(TARGETS)

.PHONY: setup clean
