#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compound.h"
#include "cell.h"

void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s CELL_FILE_NAME\n", prog_name);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		usage(argv[0]);

	char *filename = argv[1];
	struct cell cell;
	strncpy(cell.attr.filename, filename, MAX_FILENAME_LEN);
	cell_load_from_file(&cell);

	struct compound prod;
	cell_exec(&cell, &prod);

	cell_save_to_file(&cell, TRUE);

	if (cell.attr.has_retval == TRUE)
		comp_dump(&prod);

	return EXIT_SUCCESS;
}
