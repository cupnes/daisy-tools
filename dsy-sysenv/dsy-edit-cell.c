#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "common.h"

void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s OPTION CELL_FILE_NAME\n", prog_name);
	fprintf(stderr, "\n");
	fprintf(stderr, "OPTION\n");
	fprintf(stderr, "\t-f\tSet full args.\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
		usage(argv[0]);

	char *option = argv[1];
	char *cell_file_name = argv[2];

	if ((strlen(option) != 2) || (option[0] != '-'))
		usage(argv[0]);

	struct cell cell;
	strncpy(cell.attr.filename, cell_file_name, MAX_FILENAME_LEN);
	cell_load_from_file(&cell);

	switch (option[1]) {
	case 'f':
		cell.attr.has_args = cell.attr.num_args;
		break;

	default:
		usage(argv[0]);
	}

	cell_save_to_file(&cell, TRUE);

	return EXIT_SUCCESS;
}
