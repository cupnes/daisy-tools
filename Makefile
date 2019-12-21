all: dsy-sysenv

dsy-sysenv:
	make -C dsy-sysenv

clean:
	rm -f *~
	make -C dsy-sysenv clean

.PHONY: dsy-sysenv clean
