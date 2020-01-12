#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cell.h"
#include "common.h"

#define JSON_BUF_SIZE	4096

static bool_t is_verbose = FALSE;
static bool_t is_json = FALSE;

static void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s [-v] [-j] CELL_FILE_NAME\n", prog_name);
	exit(EXIT_FAILURE);
}

static void parse_option(int argc, char *argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, "vj")) != -1) {
		switch (opt) {
		case 'v':
			is_verbose = TRUE;
			break;
		case 'j':
			is_json = TRUE;
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

	if (is_json == TRUE) {
		char json_buf[JSON_BUF_SIZE];
		printf("%s", cell_make_json(&cell, is_verbose, json_buf));
	} else
		cell_dump(&cell, is_verbose);

	return EXIT_SUCCESS;
}
