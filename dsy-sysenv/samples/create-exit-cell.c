#include <string.h>

#include "../compound.h"
#include "../cell.h"
#include "../sysenv.h"
#include "../common.h"

/*
# exit(int status)	2 codes (+ 3 7)10 bytes
0x48, 0x31, 0xff,				xor	%rdi,	%rdi
0x48, 0x31, 0xc0, 0xb0, 0x3c, 0x0f, 0x05,	xor %rax,%rax	mov $0x60,%al	syscall
# return	1 code 1 byte
0xc3						ret
# All: (+ 2 1)3 codes (+ 10 1)11 bytes
 */

#define EXIT_LIFE_DURATION	100
#define EXIT_NUM_CODON	3
#define FUNC_BUF_SIZE	100

#define DEF_MUTATE_FLG(INSP_DIS, INSN_DIS, MOD_DIS, REM_DIS)	\
		.mutate_flg.insp_dis = (INSP_DIS),		\
		.mutate_flg.insn_dis = (INSN_DIS),		\
		.mutate_flg.mod_dis = (MOD_DIS),		\
		.mutate_flg.rem_dis = (REM_DIS)
#define DEF_SYSCALL_EXIT_0						\
	{								\
		.len = 3,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(FALSE, TRUE, TRUE, TRUE),		\
		.byte = {0x48, 0x31, 0xff}				\
	},								\
	{								\
		.len = 7,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(TRUE, TRUE, TRUE, TRUE),			\
		.byte = {0x48, 0x31, 0xc0, 0xb0, 0x3c, 0x0f, 0x05}	\
	}
#define DEF_RET							\
	{							\
		.len = 1,					\
		.is_buffered = FALSE,				\
		DEF_MUTATE_FLG(TRUE, TRUE, TRUE, TRUE),		\
		.byte = {0xc3}					\
	}

struct codon exit_codn[EXIT_NUM_CODON] = {
	DEF_SYSCALL_EXIT_0,
	DEF_RET
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
