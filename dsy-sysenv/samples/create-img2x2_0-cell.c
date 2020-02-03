#include <string.h>

#include "../compound.h"
#include "../cell.h"
#include "../sysenv.h"
#include "../common.h"

/* 2x2 のpbm画像
# ヘッダ
# P1
# 16 16
# ボディ
0
0
0
0
 */

#define NUM_CODONS	4
#define FILENAME	"img2x2_0"

#define FUNC_BUF_SIZE	100

#define LIFE_DURATION	100
#define FITNESS		50
#define NUM_ARGS	0
#define HAS_RETVAL	FALSE

#define DEF_MUTATE_FLG(INSP_DIS, INSN_DIS, MOD_DIS, REM_DIS)	\
		.mutate_flg.insp_dis = (INSP_DIS),		\
		.mutate_flg.insn_dis = (INSN_DIS),		\
		.mutate_flg.mod_dis = (MOD_DIS),		\
		.mutate_flg.rem_dis = (REM_DIS)
#define PIX(COLOR)						\
	{							\
		.len = 1,					\
		.is_buffered = FALSE,				\
		DEF_MUTATE_FLG(TRUE, TRUE, FALSE, TRUE),	\
		.byte = {(COLOR)}				\
	}

enum {
	WHITE = 0x30,
	BLACK
};

struct codon codn[NUM_CODONS] = {
	PIX(BLACK), PIX(WHITE),
	PIX(WHITE), PIX(WHITE)
};

void create_cell(struct codon codn[], unsigned long long num_codns,
		 char *filename)
{
	unsigned long long i;

	unsigned char func_buf[FUNC_BUF_SIZE];
	unsigned int func_size = 0;

	unsigned char *buf_p = func_buf;
	for (i = 0; i < num_codns; i++) {
		memcpy(buf_p, codn[i].byte, codn[i].len);
		buf_p += codn[i].len;
		func_size += codn[i].len;
	}

	struct cell cell;

	/* 属性情報の初期化 */
	cell.attr.life_duration = LIFE_DURATION;
	cell.attr.life_left = cell.attr.life_duration;
	cell.attr.fitness = FITNESS;
	cell.attr.num_args = NUM_ARGS;
	cell.attr.has_args = 0;
	cell.attr.has_retval = HAS_RETVAL;
	cell.attr.func_size = func_size;
	for (i = 0; i < CELL_MAX_ARGS; i++)
		cell.attr.args_buf[i] = 0;
	cell.attr.num_codns = num_codns;
	strncpy(cell.attr.filename, filename, MAX_FILENAME_LEN);

	/* DNAの初期化 */
	cell.codn_list = codn;

	/* タンパク質の初期化 */
	cell.func = (void *)func_buf;

	/* 細胞のバイナリを生成 */
	cell_save_to_file(&cell, FALSE);
}

int main(void)
{
	create_cell(codn, NUM_CODONS, FILENAME);

	return 0;
}
