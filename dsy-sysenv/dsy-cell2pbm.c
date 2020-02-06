#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "common.h"

#define PBM_FILE_DESC	"P1"

#define ARGV_PROG_NAME	argv[0]
#define ARGV_CELL_FILE_NAME	argv[1]
#define ARGV_WIDTH	argv[2]
#define ARGV_HEIGHT	argv[3]
#define ARGV_PBM_FILE_NAME	argv[4]

static void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s CELL_FILE_NAME WIDTH HEIGHT PBM_FILE_NAME\n",
		prog_name);
	exit(EXIT_FAILURE);
}

static void write_pbm_body(FILE *fp, struct cell *cell, int width)
{
	unsigned long long i;
	for (i = 0; i < cell->attr.num_codns; i++) {
		fprintf(fp, "%c", cell->codn_list[i].byte[0]);
		if ((i + 1) % width == 0)
			fprintf(fp, "\n");
		else
			fprintf(fp, " ");
	}
}

int main(int argc, char *argv[])
{
	if (argc != 5)
		usage(ARGV_PROG_NAME);

	struct cell cell;
	strncpy(cell.attr.filename, ARGV_CELL_FILE_NAME, MAX_FILENAME_LEN);
	cell_load_from_file(&cell);

	FILE *fp = fopen(ARGV_PBM_FILE_NAME, "w+b");
	if (fp == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	fprintf(fp, "%s\n", PBM_FILE_DESC);
	fprintf(fp, "%s %s\n", ARGV_WIDTH, ARGV_HEIGHT);

	write_pbm_body(fp, &cell, atoi(ARGV_WIDTH));

	fclose(fp);

	return EXIT_SUCCESS;
}
