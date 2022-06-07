#include <string.h>

#include "../compound.h"
#include "../cell.h"
#include "../sysenv.h"
#include "../common.h"

/*
0x48, 0x83, 0xc6, 0x01,				add	$1,	%rsi
0x48, 0x83, 0xc6, 0x02,				add	$2,	%rsi
...
0x48, 0x83, 0xc6, 0x5f				add	$95,	%rsi
# All: 95 codes (* 4 95)380 bytes
 */

#define CODE_FILENAME_LEN	14	/* "add_NN_rsi_MM\0" */

struct compound add_n_rsi_comp = {
	.len = 4,
	.byte = {0x48, 0x83, 0xc6, 0x00}
};

void create_add_n_rsi_code(int n, int num_each_code)
{
	add_n_rsi_comp.byte[3] = n;

	int i;
	for (i = 0; i < num_each_code; i++) {
		char s[CODE_FILENAME_LEN];
		sprintf(s, "add_%02d_rsi_%02d", n, i);
		comp_save_to_file("code/", s, &add_n_rsi_comp);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s NUM_EACH_CODE\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int n;
	for (n = 1; n <= 95; n++) {
		create_add_n_rsi_code(n, atoi(argv[1]));
	}

	return 0;
}
