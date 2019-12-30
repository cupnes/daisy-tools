#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sys/mman.h>
#include <unistd.h>

#include "compound.h"
#include "cell.h"
#include "sysenv.h"
#include "common.h"

/* 戻り値：TRUE=実行可能になった/FALSE=まだ実行不可 */
static bool_t get_args(struct cell *cell)
{
	struct compound comp;

	/* 引数の数が0でなく、全て揃っている状態でここに来るのは何かおかしい
	 * (引数が全て揃ったらそのタイミングで消費される) */
	ASSERT((cell->attr.num_args == 0)
	       || (cell->attr.has_args < cell->attr.num_args));

	/* (引数があるなら)化合物を一つ取得しバッファへ追加 */
	if (cell->attr.num_args > 0) {
		if (sysenv_get_comp(COMP_TYPE_DATA, NULL, &comp) == TRUE)
			cell->attr.args_buf[cell->attr.has_args++] = comp.int64;
	}

	/* 引数が揃っていなければ実行できない */
	bool_t is_executable;
	if ((cell->attr.num_args > 0)
	    && (cell->attr.has_args < cell->attr.num_args))
		is_executable = FALSE;
	else
		is_executable = TRUE;

	return is_executable;
}

static bool_t growth(struct cell *cell)
{
	struct compound comp;

	/* コード化合物を一つ取得 */
	if (sysenv_get_comp(COMP_TYPE_CODE, NULL, &comp) == FALSE)
		return FALSE;

	unsigned int i;

	/* 空きのあるコドンがあれば結合する */
	bool_t is_used = FALSE;
	for (i = 0; i < cell->attr.num_codns; i++) {
		if ((cell->codn_list[i].int64 == comp.int64)
		    && (cell->codn_list[i].is_buffered == FALSE)) {
			cell->codn_list[i].is_buffered = TRUE;
			is_used = TRUE;
			break;
		}
	}

	/* 結合しなかった場合、環境へ排出する */
	if (is_used == FALSE)
		sysenv_put_comp(COMP_TYPE_CODE, NULL, &comp);

	/* 細胞分裂可能か否かを判定 */
	for (i = 0; i < cell->attr.num_codns; i++) {
		if (cell->codn_list[i].is_buffered == FALSE)
			return FALSE;
	}
	return TRUE;
}

static void *central_dogma(struct cell *cell)
{
	void *func_new = mmap(
		NULL, cell->attr.func_size, PROT_EXEC | PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	ASSERT(cell->func != MAP_FAILED);

	unsigned char *new_func_p = func_new;
	unsigned int i;
	for (i = 0; i < cell->attr.num_codns; i++) {
		memcpy(new_func_p, cell->codn_list[i].byte,
		       cell->codn_list[i].len);
		cell->codn_list[i].is_buffered = FALSE;
		new_func_p += cell->codn_list[i].len;
	}

	return func_new;
}

static struct codon *copy_codon_list(struct cell *cell)
{
	struct codon *codn_new_list =
		malloc(sizeof(struct codon) * cell->attr.num_codns);
	ASSERT(codn_new_list != NULL);

	unsigned int i;
	for (i = 0; i < cell->attr.num_codns; i++) {
		codn_new_list[i].len = cell->codn_list[i].len;
		codn_new_list[i].is_buffered = FALSE;
		codn_new_list[i]._rsv = codn_new_list[i]._rsv2 = 0;
		codn_new_list[i].int64 = cell->codn_list[i].int64;
	}

	return codn_new_list;
}

static void division(struct cell *cell)
{
	unsigned int i;

	/* 細胞の構造を生成 */
	struct cell cell_new;

	/* 属性情報を遺伝 */
	cell_new.attr.life_duration = cell->attr.life_duration;
	cell_new.attr.life_left = cell_new.attr.life_duration;
	cell_new.attr.fitness = cell->attr.fitness;
	cell_new.attr.num_args = cell->attr.num_args;
	cell_new.attr.has_args = 0;
	cell_new.attr.has_retval = cell->attr.has_retval;
	cell_new.attr.func_size = cell->attr.func_size;
	for (i = 0; i < CELL_MAX_ARGS; i++)
		cell_new.attr.args_buf[i] = 0;
	cell_new.attr.num_codns = cell->attr.num_codns;
	cell_new.attr.filename[0] = '\0';

	/* DNAを複製 */
	cell_new.codn_list = copy_codon_list(cell);

	/* 現細胞のDNAから生成したタンパク質リストを新細胞へ繋ぐ */
	cell_new.func = central_dogma(cell);

	/* 新細胞を環境へ放出 */
	sysenv_put_cell(&cell_new);
}

static void death(struct cell *cell)
{
	/* 細胞の構成要素をバラバラにして環境へ放出 */
	unsigned int i;

	struct compound comp;

	/* 引数 */
	for (i = 0; i < cell->attr.has_args; i++) {
		comp.len = sizeof(comp_data_t);
		comp.int64 = cell->attr.args_buf[i];
		sysenv_put_comp(COMP_TYPE_DATA, NULL, &comp);
	}

	/* DNA */
	for (i = 0; i < cell->attr.num_codns; i++) {
		comp.len = cell->codn_list[i].len;
		comp.int64 = cell->codn_list[i].int64;
		if (cell->codn_list[i].is_buffered == TRUE) {
			sysenv_put_comp(COMP_TYPE_CODE, NULL, &comp);
		}
		sysenv_put_comp(COMP_TYPE_CODE, NULL, &comp);
	}
	free(cell->codn_list);

	/* タンパク質 */
	int _res = munmap(cell->func, cell->attr.func_size);
	ASSERT(_res == 0);

	/* ファイルを削除 */
	cell_remove_file(cell);
}

static FILE *open_cell_file(struct cell *cell, const char *mode)
{
	/* パスを作成 */
	char path[MAX_PATH_LEN + 1] = CELL_DIR_NAME;
	strncpy(&path[CELL_DIR_LEN], cell->attr.filename,
		(MAX_PATH_LEN + 1) - CELL_DIR_LEN);

	/* ファイルを開く */
	FILE *fp = fopen(path, mode);
	return fp;
}

void cell_load_from_file(struct cell *cell)
{
	/* ファイルを開く */
	FILE *load_fp = open_cell_file(cell, "rb");
	ASSERT(load_fp != NULL);

	size_t n;
	size_t read_bytes;

	/* 属性情報をロード */
	n = sizeof(struct cell_attributes) - MAX_FILENAME_LEN;
	read_bytes = fread_safe(&cell->attr, n, load_fp);
	ASSERT(read_bytes == n);

	/* DNAをロード */
	n = sizeof(struct codon) * cell->attr.num_codns;
	cell->codn_list = malloc(n);
	ASSERT(cell->codn_list != NULL);
	read_bytes = fread_safe(cell->codn_list, n, load_fp);
	ASSERT(read_bytes == n);

	/* タンパク質をロード */
	cell->func = mmap(
		NULL, cell->attr.func_size, PROT_EXEC | PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	ASSERT(cell->func != MAP_FAILED);
	read_bytes = fread_safe(cell->func, cell->attr.func_size, load_fp);
	ASSERT(read_bytes == cell->attr.func_size);

	/* ファイルを閉じる */
	fclose(load_fp);
}

void cell_save_to_file(struct cell *cell, bool_t do_free)
{
	/* ファイルを新規作成(存在する場合は上書き) */
	FILE *save_fp = open_cell_file(cell, "w+b");
	ASSERT(save_fp != NULL);

	size_t n;
	size_t write_bytes;
	int _res;

	/* 属性情報をセーブ */
	n = sizeof(struct cell_attributes) - MAX_FILENAME_LEN;
	write_bytes = fwrite_safe(&cell->attr, n, save_fp);
	ASSERT(write_bytes == n);

	/* DNAをセーブ */
	n = sizeof(struct codon) * cell->attr.num_codns;
	write_bytes = fwrite_safe(cell->codn_list, n, save_fp);
	ASSERT(write_bytes == n);
	if (do_free == TRUE)
		free(cell->codn_list);

	/* タンパク質をセーブ */
	write_bytes = fwrite_safe(cell->func, cell->attr.func_size, save_fp);
	ASSERT(write_bytes == cell->attr.func_size);
	if (do_free == TRUE) {
		_res = munmap(cell->func, cell->attr.func_size);
		ASSERT(_res == 0);
	}

	/* ファイルを閉じる */
	int fd = fileno(save_fp);
	ASSERT(fd != -1);
	_res = fsync(fd);
	ASSERT(_res != -1);
	fclose(save_fp);
}

void cell_remove_file(struct cell *cell)
{
	/* パスを作成 */
	char path[MAX_PATH_LEN + 1] = CELL_DIR_NAME;
	strncpy(&path[CELL_DIR_LEN], cell->attr.filename,
		(MAX_PATH_LEN + 1) - CELL_DIR_LEN);

	/* 削除 */
	ASSERT(remove(path) == 0);
}

void cell_do_cycle(char *filename)
{
	/* ファイルをロード */
	struct cell cell;
	strncpy(cell.attr.filename, filename, MAX_FILENAME_LEN);
	cell_load_from_file(&cell);

	/* 代謝/運動 */
	bool_t is_executable = get_args(&cell);
	if (is_executable == TRUE) {
		cell_save_to_file(&cell, TRUE);
		sysenv_exec_and_eval(&cell);
		cell_load_from_file(&cell);
	}

	/* 成長 */
	bool_t is_divisible = growth(&cell);
	if (is_divisible == TRUE) {
		/* 増殖 */
		division(&cell);
	}

	/* 寿命を減らす */
	cell.attr.life_left--;
	if (cell.attr.life_left == 0) {
		/* 死 */
		death(&cell);
	} else {
		/* ファイルへセーブ */
		cell_save_to_file(&cell, TRUE);
	}
}

/* 関数を実行した場合TRUEを、しなかった場合FALSEを返す */
void cell_exec(struct cell *cell, struct compound *prod)
{
	/* 関数を実行 */
	prod->int64 = cell->func(
		cell->attr.args_buf[0], cell->attr.args_buf[1],
		cell->attr.args_buf[2], cell->attr.args_buf[3]);
	prod->len = sizeof(comp_data_t);

	/* 引数を消費 */
	cell->attr.has_args = 0;

	/* 戻り値が無いならここで終了 */
	if (cell->attr.has_retval == FALSE)
		return;

	/* 戻り値で化合物ファイルを生成し環境へ放出 */
	sysenv_put_comp(COMP_TYPE_DATA, NULL, prod);
}

void cell_dump(struct cell *cell)
{
	unsigned int i, j;

	printf("[Attributes]\n");
	printf("- life_duration\t: %d\n", cell->attr.life_duration);
	printf("- life_left\t: %d\n", cell->attr.life_left);
	printf("- fitness\t: %d\n", cell->attr.fitness);
	printf("- num_args\t: %d\n", cell->attr.num_args);
	printf("- has_args\t: %d\n", cell->attr.has_args);
	printf("- has_retval\t: %d\n", cell->attr.has_retval);
	printf("- func_size\t: %d\n", cell->attr.func_size);
	printf("- args_buf\t:\n");
	for (i = 0; i < CELL_MAX_ARGS; i++)
		printf("  [%d] 0x%016llx\n", i, cell->attr.args_buf[i]);
	printf("- num_codns\t: %lld\n", cell->attr.num_codns);

	printf("\n");

	printf("[DNA]\n");
	if (cell->codn_list == NULL)
		printf("- codn_list\t: NULL\n");
	else {
		for (i = 0; i < cell->attr.num_codns; i++) {
			printf("- codn_list[%d]\t:\n", i);
			printf("  .len\t = %d\n", cell->codn_list[i].len);
			printf("  .is_buffered\t = %d\n",
			       cell->codn_list[i].is_buffered);
			printf("  ._rsv\t = 0x%04x\n", cell->codn_list[i]._rsv);
			printf("  ._rsv2\t = 0x%08x\n",
			       cell->codn_list[i]._rsv2);
			printf("  .int64\t = 0x%016llx\n",
			       cell->codn_list[i].int64);
			printf("  .byte\t = 0x");
			for (j = 0; j < 8; j++)
				printf(" %02x", cell->codn_list[i].byte[j]);
			printf("\n");
		}
	}

	printf("\n");

	printf("[Protein]\n");
	if (cell->func == NULL)
		printf("- func\t: NULL\n");
	else {
		printf("- func\t: 0x\n");
		printf(" ");
		unsigned char *p = (unsigned char *)cell->func;
		for (i = 0; i < cell->attr.func_size; i++)
			printf(" %02x", p[i]);
	}
	printf("\n");
}
