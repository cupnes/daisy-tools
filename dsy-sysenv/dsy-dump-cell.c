#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "common.h"

void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s CELL_FILE_NAME\n", prog_name);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		usage(argv[0]);

	struct cell cell;
	strncpy(cell.attr.filename, argv[1], MAX_FILENAME_LEN);
	cell_load_from_file(&cell);
	cell_dump(&cell);

	return EXIT_SUCCESS;
}