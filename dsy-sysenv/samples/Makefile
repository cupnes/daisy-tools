TARGETS = create-init-cell create-incr-cell-data create-exit-cell
CFLAGS = -Wall -Wextra -static
COMMON_SRCS = ../sysenv.c ../cell.c ../compound.c ../common.c
WORK_DIR ?= $(HOME)/dsy-work

all: $(TARGETS)

create-init-cell: create-init-cell.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-incr-cell-data: create-incr-cell-data.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

create-exit-cell: create-exit-cell.c $(COMMON_SRCS)
	gcc $(CFLAGS) -o $@ $+

setup: $(TARGETS) create-init-codes dsy-name dsy-eval-init		\
	dsy-eval-incr cell-fitness-logger dsy-log2hist dsy-show-hists
	mkdir -p $(WORK_DIR)/samples
	cp $+ $(WORK_DIR)/samples/

clean:
	rm -f *~ $(TARGETS)

.PHONY: setup clean
