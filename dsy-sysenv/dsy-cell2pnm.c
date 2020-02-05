#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "common.h"

#define ARGV_PROG_NAME	argv[0]
#define ARGV_CELL_FILE_NAME	argv[1]
#define ARGV_PNM_TYPE	argv[2]
#define ARGV_WIDTH	argv[3]
#define ARGV_HEIGHT	argv[4]
#define ARGV_PNM_FILE_NAME	argv[5]

static void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s CELL_FILE_NAME PNM_TYPE WIDTH HEIGHT"
		" PNM_FILE_NAME\n", prog_name);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if (argc != 6)
		usage(ARGV_PROG_NAME);

	struct cell cell;
	strncpy(cell.attr.filename, ARGV_CELL_FILE_NAME, MAX_FILENAME_LEN);
	cell_load_from_file(&cell);

	FILE *fp = fopen(ARGV_PNM_FILE_NAME, "w+b");
	if (fp == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	fprintf(fp, "%s\n", ARGV_PNM_TYPE);
	fprintf(fp, "%s %s\n", ARGV_WIDTH, ARGV_HEIGHT);

	fclose(fp);

	return EXIT_SUCCESS;
}
