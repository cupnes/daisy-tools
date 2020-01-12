#include <string.h>

#include "../compound.h"
#include "../cell.h"
#include "../sysenv.h"
#include "../common.h"

/* exit(2)
0x48, 0x31, 0xff,				xor	%rdi,	%rdi
0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00,	mov	$60,	%rax
0x0f, 0x05,					syscall
0xc3						ret
 */

#define EXIT_LIFE_DURATION	100
#define EXIT_NUM_CODON	4
#define FUNC_BUF_SIZE	100

struct codon exit_codn[EXIT_NUM_CODON] = {
	/* { */
	/* 	.len = 7, */
	/* 	.is_buffered = FALSE, */
	/* 	.byte = {0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00} */
	/* 	/\* mov    $0x1,%rdi *\/ */
	/* }, */
	{
		.len = 3,
		.is_buffered = FALSE,
		.byte = {0x48, 0x31, 0xff}
		/* xor	%rdi,	%rdi */
	},
	{
		.len = 7,
		.is_buffered = FALSE,
		.byte = {0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00}
		/* mov	$60,	%rax */
	},
	{
		.len = 2,
		.is_buffered = FALSE,
		.byte = {0x0f, 0x05}
		/* syscall */
	},
	{
		.len = 1,
		.is_buffered = FALSE,
		.byte = {0xc3}
		/* ret */
	}
};

void create_exit_cell(void)
{
	unsigned char i;

	unsigned char func_buf[FUNC_BUF_SIZE];
	unsigned int exit_func_size = 0;

	unsigned char *buf_p = func_buf;
	for (i = 0; i < EXIT_NUM_CODON; i++) {
		memcpy(buf_p, exit_codn[i].byte, exit_codn[i].len);
		buf_p += exit_codn[i].len;
		exit_func_size += exit_codn[i].len;
	}

	struct cell cell;

	/* 属性情報の初期化 */
	cell.attr.life_duration = EXIT_LIFE_DURATION;
	cell.attr.life_left = cell.attr.life_duration;
	cell.attr.fitness = 50;
	cell.attr.num_args = 0;
	cell.attr.has_args = 0;
	cell.attr.has_retval = FALSE;
	cell.attr.func_size = exit_func_size;
	for (i = 0; i < CELL_MAX_ARGS; i++)
		cell.attr.args_buf[i] = 0;
	cell.attr.num_codns = EXIT_NUM_CODON;
	strncpy(cell.attr.filename, "exit", MAX_FILENAME_LEN);

	/* DNAの初期化 */
	cell.codn_list = exit_codn;

	/* タンパク質の初期化 */
	cell.func = (void *)func_buf;

	/* 細胞のバイナリを生成 */
	cell_save_to_file(&cell, FALSE);
}

int main(void)
{
	create_exit_cell();

	return 0;
}
