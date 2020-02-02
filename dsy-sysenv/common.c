#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "common.h"
#include "cell.h"

size_t fread_safe(void *ptr, size_t size, FILE *stream)
{
	size_t read_bytes = 0;
	unsigned char *p = ptr;
	while (read_bytes < size) {
		size_t n = fread(p, 1, size - read_bytes, stream);
		ASSERT(ferror(stream) == 0);
		p += n;
		read_bytes += n;
		if (feof(stream) != 0)
			break;
	}
	return read_bytes;
}

size_t fwrite_safe(void *ptr, size_t size, FILE *stream)
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

void close_namelist(struct dirent **namelist, int num_files)
{
	int i;
	for (i = 0; i < num_files; i++) {
		free(namelist[i]);
	}
	free(namelist);
}
