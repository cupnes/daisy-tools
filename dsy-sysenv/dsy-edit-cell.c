#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "common.h"

#define MAX_BUF_LEN	100

static void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s OPTION CELL_FILE_NAME\n", prog_name);
	fprintf(stderr, "\n");
	fprintf(stderr, "OPTION\n");
	fprintf(stderr, "\t--fitness={0..100}\t\tSet .attr.fitness\n");
	fprintf(stderr, "\t--has_args={0..num_args}\tSet .attr.has_args\n");
	exit(EXIT_FAILURE);
}

static void option_parser(const char *option_and_value, struct cell *cell)
{
	char *delim_ptr = strchr(option_and_value, '=');
	ASSERT(delim_ptr != NULL);

	size_t option_len = delim_ptr - option_and_value;
	ASSERT(option_len > 0);
	ASSERT((option_len + 1) <= CELL_MAX_MEMBER_NAME_LEN);

	char option[CELL_MAX_MEMBER_NAME_LEN];
	strncpy(option, option_and_value, option_len);
	option[option_len] = '\0';

	char *value_ptr = delim_ptr + 1;
	size_t value_len = strlen(value_ptr);
	ASSERT(value_len > 0);
	ASSERT((value_len + 1) <= CELL_MAX_MEMBER_VALUE_LEN);

	if (!strcmp(option, "--fitness"))
		cell->attr.fitness = atoi(value_ptr);
	else if (!strcmp(option, "--has_args"))
		cell->attr.has_args = atoi(value_ptr);
	else
		ERROR_WITH(TRUE, "Invalid option.");
}

int main(int argc, char *argv[])
{
	if (argc < 3)
		usage(argv[0]);

	struct cell cell;
	strncpy(cell.attr.filename, argv[argc - 1], MAX_FILENAME_LEN);
	cell_load_from_file(&cell);

	int i;
	for (i = 1; i < (argc - 1); i++)
		option_parser(argv[i], &cell);

	cell_dump(&cell);

	bool_t is_yes = FALSE;
	while (TRUE) {
		printf("Is this ok? [y/n] ");
		char _buf[MAX_BUF_LEN];
		char *_res = fgets(_buf, MAX_BUF_LEN, stdin);
		ASSERT(_res != NULL);
		if (_buf[0] == 'y') {
			is_yes = TRUE;
			break;
		} else if (_buf[0] == 'n') {
			break;
		}
	}

	if (is_yes == TRUE) {
		cell_save_to_file(&cell, TRUE);
		printf("cell \"%s\" was updated.\n", cell.attr.filename);
	}

	return EXIT_SUCCESS;
}
