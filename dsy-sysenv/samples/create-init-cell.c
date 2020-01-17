#include <string.h>

#include "../compound.h"
#include "../cell.h"
#include "../sysenv.h"
#include "../common.h"

/* 初期細胞(retのみ)
protein:                  Opcode=0xc3				ret
 */

#define INIT_LIFE_DURATION	100
#define INIT_NUM_CODON	1
#define INIT_FUNC_SIZE	1

struct codon init_codn[INIT_NUM_CODON] = {
	{
		.len = 1,
		.is_buffered = FALSE,
		.mutate_flg.insp_dis = FALSE,
		.mutate_flg.insn_dis = TRUE,
		.mutate_flg.mod_dis = TRUE,
		.mutate_flg.rem_dis = TRUE,
		.byte = {0xc3}
	}
};

unsigned char init_func_data[INIT_FUNC_SIZE] = {
	0xc3
};

void create_init_cell(void)
{
	struct cell cell;

	/* 属性情報の初期化 */
	cell.attr.life_duration = INIT_LIFE_DURATION;
	cell.attr.life_left = cell.attr.life_duration;
	cell.attr.fitness = 50;
	cell.attr.num_args = 0;
	cell.attr.has_args = 0;
	cell.attr.has_retval = FALSE;
	cell.attr.func_size = INIT_FUNC_SIZE;
	unsigned char i;
	for (i = 0; i < CELL_MAX_ARGS; i++)
		cell.attr.args_buf[i] = 0;
	cell.attr.num_codns = INIT_NUM_CODON;
	strncpy(cell.attr.filename, "initial", MAX_FILENAME_LEN);

	/* DNAの初期化 */
	cell.codn_list = init_codn;

	/* タンパク質の初期化 */
	cell.func = (void *)init_func_data;

	/* 細胞のバイナリを生成 */
	cell_save_to_file(&cell, FALSE);
}

int main(void)
{
	create_init_cell();

	return 0;
}
