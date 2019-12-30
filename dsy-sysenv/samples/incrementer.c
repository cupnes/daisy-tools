#include <string.h>

#include "../compound.h"
#include "../cell.h"
#include "../sysenv.h"
#include "../common.h"

/* インクリメンタ細胞の機械語コード
protein0: REX Prefix=0x48, Opcode=0x89, Operand(ModR/M)=0xf8	mov %rdi,%rax
protein1: REX Prefix=0x48, Opcode=0xff, Operand(ModR/M)=0xc0	inc %rax
protein2:                  Opcode=0xc3				ret
 */

#define INCR_LIFE_DURATION	100
#define INCR_NUM_CODON	3
#define INCR_FUNC_SIZE	7

struct codon incr_codn[INCR_NUM_CODON] = {
	{
		.len = 3,
		.is_buffered = FALSE,
		.byte = {0x48, 0x89, 0xf8}
	},
	{
		.len = 3,
		.is_buffered = FALSE,
		.byte = {0x48, 0xff, 0xc0}
	},
	{
		.len = 1,
		.is_buffered = FALSE,
		.byte = {0xc3}
	}
};

unsigned char incr_func_data[INCR_FUNC_SIZE] = {
	0x48, 0x89, 0xf8,
	0x48, 0xff, 0xc0,
	0xc3
};

void create_incr_cell(void)
{
	struct cell cell;

	/* 属性情報の初期化 */
	cell.attr.life_duration = INCR_LIFE_DURATION;
	cell.attr.life_left = cell.attr.life_duration;
	cell.attr.fitness = 100;
	cell.attr.num_args = 1;
	cell.attr.has_args = 0;
	cell.attr.has_retval = TRUE;
	cell.attr.func_size = INCR_FUNC_SIZE;
	unsigned char i;
	for (i = 0; i < CELL_MAX_ARGS; i++)
		cell.attr.args_buf[i] = 0;
	cell.attr.num_codns = INCR_NUM_CODON;
	strncpy(cell.attr.filename, "incrementer", MAX_FILENAME_LEN);

	/* DNAの初期化 */
	cell.codn_list = incr_codn;

	/* タンパク質の初期化 */
	cell.func = (void *)incr_func_data;

	/* 細胞のバイナリを生成 */
	cell_save_to_file(&cell, FALSE);
}

void create_data_comp(unsigned long long val)
{
	struct compound comp;

	comp.len = sizeof(comp_data_t);
	comp.int64 = val;

	comp_save_to_file(COMP_DATA_DIR_NAME, "data", &comp);
}

int main(void)
{
	create_incr_cell();

	create_data_comp(0);

	return 0;
}
