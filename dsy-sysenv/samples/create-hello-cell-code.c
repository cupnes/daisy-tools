#include <string.h>

#include "../compound.h"
#include "../cell.h"
#include "../sysenv.h"
#include "../common.h"

/*
# write(int fd, const void *buf, size_t count): H	6 insts 34 bytes
0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdi
0x48, 0xc7, 0xc6, 0x78, 0x00, 0x40, 0x00,	mov	$0x400078,	%rsi
0x48, 0x83, 0xc6, 0x29,				add	$41,	%rsi
0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdx
0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rax
0x0f, 0x05,					syscall
# write(2): e	6 insts 34 bytes
0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdi
0x48, 0xc7, 0xc6, 0x78, 0x00, 0x40, 0x00,	mov	$0x400078,	%rsi
0x48, 0x83, 0xc6, 0x46,				add	$70,	%rsi
0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdx
0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rax
0x0f, 0x05,					syscall
# write(2): l	6 insts 34 bytes
0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdi
0x48, 0xc7, 0xc6, 0x78, 0x00, 0x40, 0x00,	mov	$0x400078,	%rsi
0x48, 0x83, 0xc6, 0x4d,				add	$77,	%rsi
0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdx
0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rax
0x0f, 0x05,					syscall
# write(2): l	6 insts 34 bytes
0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdi
0x48, 0xc7, 0xc6, 0x78, 0x00, 0x40, 0x00,	mov	$0x400078,	%rsi
0x48, 0x83, 0xc6, 0x4d,				add	$0x77,	%rsi
0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdx
0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rax
0x0f, 0x05,					syscall
# write(2): o	6 insts 34 bytes
0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdi
0x48, 0xc7, 0xc6, 0x78, 0x00, 0x40, 0x00,	mov	$0x400078,	%rsi
0x48, 0x83, 0xc6, 0x50,				add	$80,	%rsi
0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rdx
0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,	mov	$1,	%rax
0x0f, 0x05,					syscall
# exit(int status)	3 insts 12 bytes
0x48, 0x31, 0xff,				xor	%rdi,	%rdi
0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00,	mov	$60,	%rax
0x0f, 0x05,					syscall
# return	1 inst 1 byte
0xc3						ret
 */

#define HELLO_LIFE_DURATION	100
#define HELLO_NUM_CODON	34	/* (+ (* 6 5) 3 1)34 */
#define FUNC_BUF_SIZE	200	/* (+ (* 34 5) 12 1)183 */
#define START_ASCII_SPACE	0x00400078

#define NUM_EACH_CODE	100
#define CODE_FILENAME_LEN	12	/* "hello_NN_MM\0" */

#define DEF_MUTATE_FLG(INSP_DIS, INSN_DIS, MOD_DIS, REM_DIS)	\
		.mutate_flg.insp_dis = (INSP_DIS),		\
		.mutate_flg.insn_dis = (INSN_DIS),		\
		.mutate_flg.mod_dis = (MOD_DIS),		\
		.mutate_flg.rem_dis = (REM_DIS)
#define DEF_SYSCALL_WRITE_A_ASCII_CODONS(INSP_DIS, C)			\
	{								\
		.len = 7,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG((INSP_DIS), FALSE, FALSE, FALSE),	\
		.byte = {0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00}	\
	},								\
	{								\
		.len = 7,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(FALSE, FALSE, FALSE, FALSE),		\
		.byte = {0x48, 0xc7, 0xc6, 0x78, 0x00, 0x40, 0x00}	\
	},								\
	{								\
		.len = 4,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(FALSE, FALSE, FALSE, FALSE),		\
		.byte = {0x48, 0x83, 0xc6, (C)}				\
	},								\
	{								\
		.len = 7,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(FALSE, FALSE, FALSE, FALSE),		\
		.byte = {0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00}	\
	},								\
	{								\
		.len = 7,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(FALSE, FALSE, FALSE, FALSE),		\
		.byte = {0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00}	\
	},								\
	{								\
		.len = 2,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(FALSE, FALSE, FALSE, FALSE),		\
		.byte = {0x0f, 0x05}					\
	}
#define DEF_SYSCALL_EXIT_0						\
	{								\
		.len = 3,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(FALSE, FALSE, FALSE, FALSE),		\
		.byte = {0x48, 0x31, 0xff}				\
		/* xor	%rdi,	%rdi */					\
	},								\
	{								\
		.len = 7,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(FALSE, FALSE, FALSE, FALSE),		\
		.byte = {0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00}	\
		/* mov	$60,	%rax */					\
	},								\
	{								\
		.len = 2,						\
		.is_buffered = FALSE,					\
		DEF_MUTATE_FLG(FALSE, FALSE, FALSE, FALSE),		\
		.byte = {0x0f, 0x05}					\
		/* syscall */						\
	}
#define DEF_RET							\
	{							\
		.len = 1,					\
		.is_buffered = FALSE,				\
		 DEF_MUTATE_FLG(FALSE, FALSE, FALSE, FALSE),	\
		.byte = {0xc3}					\
		/* ret */					\
	}

struct codon hello_codn[HELLO_NUM_CODON] = {
	DEF_SYSCALL_WRITE_A_ASCII_CODONS(TRUE, 0x29),	/* 'H' */
	DEF_SYSCALL_WRITE_A_ASCII_CODONS(FALSE, 0x46),	/* 'e' */
	DEF_SYSCALL_WRITE_A_ASCII_CODONS(FALSE, 0x4d),	/* 'l' */
	DEF_SYSCALL_WRITE_A_ASCII_CODONS(FALSE, 0x4d),	/* 'l' */
	DEF_SYSCALL_WRITE_A_ASCII_CODONS(FALSE, 0x50),	/* 'o' */
	DEF_SYSCALL_EXIT_0,
	DEF_RET
};

void create_hello_cell(void)
{
	unsigned char i;

	unsigned char func_buf[FUNC_BUF_SIZE];
	unsigned int hello_func_size = 0;

	unsigned char *buf_p = func_buf;
	for (i = 0; i < HELLO_NUM_CODON; i++) {
		memcpy(buf_p, hello_codn[i].byte, hello_codn[i].len);
		buf_p += hello_codn[i].len;
		hello_func_size += hello_codn[i].len;
	}

	struct cell cell;

	/* 属性情報の初期化 */
	cell.attr.life_duration = HELLO_LIFE_DURATION;
	cell.attr.life_left = cell.attr.life_duration;
	cell.attr.fitness = 50;
	cell.attr.num_args = 0;
	cell.attr.has_args = 0;
	cell.attr.has_retval = FALSE;
	cell.attr.func_size = hello_func_size;
	for (i = 0; i < CELL_MAX_ARGS; i++)
		cell.attr.args_buf[i] = 0;
	cell.attr.num_codns = HELLO_NUM_CODON;
	strncpy(cell.attr.filename, "hello", MAX_FILENAME_LEN);

	/* DNAの初期化 */
	cell.codn_list = hello_codn;

	/* タンパク質の初期化 */
	cell.func = (void *)func_buf;

	/* 細胞のバイナリを生成 */
	cell_save_to_file(&cell, FALSE);
}

void create_hello_code(void)
{
	unsigned int i;
	for (i = 0; i < HELLO_NUM_CODON; i++) {
		struct compound comp;
		comp.len = hello_codn[i].len;
		comp.int64 = hello_codn[i].int64;

		unsigned int j;
		for (j = 0; j < NUM_EACH_CODE; j++) {
			char s[CODE_FILENAME_LEN];
			sprintf(s, "hello_%02d_%02d", i, j);
			comp_save_to_file("code/", s, &comp);
		}
	}
}

int main(void)
{
	create_hello_cell();
	create_hello_code();

	return 0;
}
