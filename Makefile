CFLAGS = -Wall -Wextra -g
# CFLAGS += -static
BINS = bin/dsy-sysenv bin/dsy-exec-cell bin/dsy-dump-cell	\
	bin/dsy-edit-cell bin/dsy-cell2elf bin/dsy-cell2pbm	\
	bin/dsy-dump-ascii-list
WORK_DIR ?= $(HOME)/dsy-work

all: $(BINS) samples

bin/dsy-sysenv: main.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

bin/dsy-exec-cell: dsy-exec-cell.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

bin/dsy-dump-cell: dsy-dump-cell.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

bin/dsy-edit-cell: dsy-edit-cell.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

bin/dsy-cell2elf: dsy-cell2elf.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

bin/dsy-cell2pbm: dsy-cell2pbm.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

bin/dsy-dump-ascii-list: dsy-dump-ascii-list.c sysenv.c cell.c compound.c common.c
	gcc $(CFLAGS) -o $@ $+

samples:
	make -C $@

busybox-static:
	apt-get download $@
	dpkg -x busybox-static*.deb $@

setup: all
	mkdir -p $(WORK_DIR)/cell
	mkdir -p $(WORK_DIR)/code
	mkdir -p $(WORK_DIR)/data
	cp -r bin $(WORK_DIR)/
	make -C samples setup WORK_DIR=$(WORK_DIR)

setup_rootfs: all busybox-static
	mkdir -p $(WORK_DIR)/cell
	mkdir -p $(WORK_DIR)/code
	mkdir -p $(WORK_DIR)/data
	cp -r bin $(WORK_DIR)/
	cp busybox-static/bin/busybox $(WORK_DIR)/bin/
	ln -sf busybox $(WORK_DIR)/bin/sh
	make -C samples setup WORK_DIR=$(WORK_DIR)

run:
	sudo chroot --userspec=$(shell id -u):$(shell id -g) $(WORK_DIR) dsy-sysenv

sh:
	sudo unshare -pf chroot --userspec=$(shell id -u):$(shell id -g) $(WORK_DIR) sh

sush:
	sudo unshare -pf chroot $(WORK_DIR) sh

clean:
	rm -rf *~ bin/*~ tools/*~ $(BINS) busybox-static*
	make -C samples clean

.PHONY: samples setup run sh sush clean
