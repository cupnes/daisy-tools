TARGETS = create-init-cell create-incr-cell-data create-exit-cell	\
		create-exit_1-cell create-puta-codes			\
		create-puta-cell-code create-hello-cell-code		\
		create-img2x2_0-cell					\
		create-add_1_rsi-add_95_rsi-codes			\
		create-add_1_rsi-codes create-sub_1_rsi-codes
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

create-puta-codes: create-puta-codes.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-puta-cell-code: create-puta-cell-code.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-hello-cell-code: create-hello-cell-code.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-img2x2_0-cell: create-img2x2_0-cell.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-add_1_rsi-add_95_rsi-codes: create-add_1_rsi-add_95_rsi-codes.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-add_1_rsi-codes: create-add_1_rsi-codes.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-sub_1_rsi-codes: create-sub_1_rsi-codes.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

setup: $(TARGETS) create-exit-codes create-exit_1-codes			\
	create-mov_1_rdi-codes create-syscall-code dsy-name		\
	dsy-eval-init dsy-eval-exit dsy-eval-exit_0			\
	dsy-eval-exit_0-unshare dsy-eval-exit_1 dsy-eval-incr		\
	dsy-eval-puta dsy-eval-puta_0 dsy-eval-hello setup-sample
	mkdir -p $(WORK_DIR)/samples
	cp $+ $(WORK_DIR)/samples/

clean:
	rm -f *~ $(TARGETS)

.PHONY: setup clean
