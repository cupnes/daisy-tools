#include <stdio.h>
#include <stdlib.h>

#include "cell.h"

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
	cell_load_from_file(argv[1], &cell);
	cell_dump(&cell);
	cell_save_to_file(argv[1], &cell, TRUE);

	return EXIT_SUCCESS;
}
