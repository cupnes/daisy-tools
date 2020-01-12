#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cell.h"
#include "common.h"

static bool_t is_verbose = FALSE;

static void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s [-v] CELL_FILE_NAME\n", prog_name);
	exit(EXIT_FAILURE);
}

static void parse_option(int argc, char *argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, "v")) != -1) {
		switch (opt) {
		case 'v':
			is_verbose = TRUE;
			break;
		default:
			usage(argv[0]);
		}
	}
}

int main(int argc, char *argv[])
{
	parse_option(argc, argv);

	if (optind >= argc)
		usage(argv[0]);

	struct cell cell;
	strncpy(cell.attr.filename, argv[optind], MAX_FILENAME_LEN);
	cell_load_from_file(&cell);
	cell_dump(&cell, is_verbose);

	return EXIT_SUCCESS;
}
