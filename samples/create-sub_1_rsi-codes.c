#include <string.h>

#include "../compound.h"
#include "../cell.h"
#include "../sysenv.h"
#include "../common.h"

/*
0x48, 0x83, 0xee, 0x01,				sub	$1,	%rsi
# All: 1 codes 4 bytes
 */

#define CODE_FILENAME_LEN	16	/* "sub_1_rsi_MMMMM\0" */

struct compound sub_1_rsi_comp = {
	.len = 4,
	.byte = {0x48, 0x83, 0xee, 0x01}
};

void create_sub_1_rsi_code(int num_each_code)
{
	int i;
	for (i = 0; i < num_each_code; i++) {
		char s[CODE_FILENAME_LEN];
		sprintf(s, "sub_1_rsi_%05d", i);
		comp_save_to_file("code/", s, &sub_1_rsi_comp);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s NUM_EACH_CODE\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	create_sub_1_rsi_code(atoi(argv[1]));

	return 0;
}
