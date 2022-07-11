#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <err.h>

#define ERR_FMT		"at %s:%d %s()",	__FILE__, __LINE__, __FUNCTION__
#define ASSERT(X)	if (!(X)) err(EXIT_FAILURE, ERR_FMT)

#define PHNUM	1
#define LOAD_VADDR	0x400000

static void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s CODE_SIZE OUTPUT_FILE_NAME\n", prog_name);
	exit(EXIT_FAILURE);
}

static size_t fwrite_safe(void *ptr, size_t size, FILE *stream)
{
	size_t write_bytes = 0;
	unsigned char *p = ptr;
	while (write_bytes < size) {
		size_t n = fwrite(p, 1, size - write_bytes, stream);
		ASSERT(ferror(stream) == 0);
		p += n;
		write_bytes += n;
		if (feof(stream) != 0)
			break;
	}
	return write_bytes;
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
		LOAD_VADDR + sizeof(Elf64_Ehdr) + (sizeof(Elf64_Phdr) * PHNUM);

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
		+ load_data_size;
	phdr.p_memsz = phdr.p_filesz;
	phdr.p_align = 0x200000;

	size_t n = fwrite_safe(&phdr, sizeof(Elf64_Phdr), fp);
	if (n != sizeof(Elf64_Phdr)) {
		perror("fwrite(phdr)");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 3)
		usage(argv[0]);

	int code_size = atoi(argv[1]);
	if (code_size < 0) {
		fprintf(stderr, "Error: Invalid CODE_SIZE %d. CODE_SIZE must be"
			" a positive number.\n", code_size);
		exit(EXIT_FAILURE);
	}

	FILE *fp = fopen(argv[2], "w+b");
	if (fp == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	write_elf64_ehdr(fp);
	write_elf64_phdr(fp, code_size);

	fclose(fp);

	return EXIT_SUCCESS;
}
