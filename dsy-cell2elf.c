#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "cell.h"
#include "common.h"

#define PHNUM	1
#define LOAD_VADDR	0x400000

static char load_data_char_list[] =
	"\n !\"#$%&'()*+,-./0123456789:;<=>?@"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"[\\]^_`"
	"abcdefghijklmnopqrstuvwxyz"
	"{|}~";
static unsigned char load_data_char_list_len;

static void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s CELL_FILE_NAME ELF_FILE_NAME\n", prog_name);
	exit(EXIT_FAILURE);
}

static void write_elf64_ehdr(FILE *fp)
{
	Elf64_Ehdr ehdr;
	memcpy(ehdr.e_ident, ELFMAG, SELFMAG);
	ehdr.e_ident[EI_CLASS] = ELFCLASS64;
	ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr.e_ident[EI_VERSION] = EV_CURRENT;
	ehdr.e_ident[EI_OSABI] = ELFOSABI_SYSV;
	ehdr.e_ident[EI_ABIVERSION] = 0;
	unsigned char i;
	for (i = EI_PAD; i < EI_NIDENT; i++)
		ehdr.e_ident[i] = 0;
	ehdr.e_type = ET_EXEC;
	ehdr.e_machine = EM_X86_64;
	ehdr.e_version = EV_CURRENT;
	ehdr.e_phoff = sizeof(Elf64_Ehdr);
	ehdr.e_shoff = 0;
	ehdr.e_flags = 0;
	ehdr.e_ehsize = sizeof(Elf64_Ehdr);
	ehdr.e_phentsize = sizeof(Elf64_Phdr);
	ehdr.e_phnum = PHNUM;
	ehdr.e_shentsize = 0;
	ehdr.e_shnum = 0;
	ehdr.e_shstrndx = SHN_UNDEF;

	ehdr.e_entry =
		LOAD_VADDR + sizeof(Elf64_Ehdr) + (sizeof(Elf64_Phdr) * PHNUM)
		+ load_data_char_list_len;

	size_t n = fwrite_safe(&ehdr, sizeof(Elf64_Ehdr), fp);
	if (n != sizeof(Elf64_Ehdr)) {
		perror("fwrite(ehdr)");
		exit(EXIT_FAILURE);
	}
}

static void write_elf64_phdr(FILE *fp, unsigned int load_data_size)
{
	Elf64_Phdr phdr;
	phdr.p_type = PT_LOAD;
	phdr.p_flags = PF_X | PF_R;
	phdr.p_offset = 0;
	phdr.p_vaddr = LOAD_VADDR;
	phdr.p_paddr = phdr.p_vaddr;
	phdr.p_filesz = sizeof(Elf64_Ehdr) + (sizeof(Elf64_Phdr) * PHNUM)
		+ load_data_char_list_len + load_data_size;
	phdr.p_memsz = phdr.p_filesz;
	phdr.p_align = 0x200000;

	size_t n = fwrite_safe(&phdr, sizeof(Elf64_Phdr), fp);
	if (n != sizeof(Elf64_Phdr)) {
		perror("fwrite(phdr)");
		exit(EXIT_FAILURE);
	}
}

static void write_load_data(
	FILE *fp, void *load_data, unsigned int load_data_size)
{
	size_t n;

	n = fwrite_safe(load_data_char_list, load_data_char_list_len, fp);
	if (n != load_data_char_list_len) {
		perror("fwrite(char list)");
		exit(EXIT_FAILURE);
	}

	n = fwrite_safe(load_data, load_data_size, fp);
	if (n != load_data_size) {
		perror("fwrite(load data)");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 3)
		usage(argv[0]);

	load_data_char_list_len = strlen(load_data_char_list);

	struct cell cell;
	strncpy(cell.attr.filename, argv[1], MAX_FILENAME_LEN);
	cell_load_from_file(&cell);

	FILE *fp = fopen(argv[2], "w+b");
	if (fp == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	write_elf64_ehdr(fp);
	write_elf64_phdr(fp, cell.attr.func_size);
	write_load_data(fp, cell.func, cell.attr.func_size);

	fclose(fp);

	return EXIT_SUCCESS;
}
