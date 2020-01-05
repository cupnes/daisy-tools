#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sys/mman.h>
#include <unistd.h>
#include <syslog.h>

#include "compound.h"
#include "cell.h"
#include "sysenv.h"
#include "common.h"

#define CELL_STR_BUF_SIZE	4096

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
		if (sysenv_get_comp(cell, COMP_TYPE_DATA, NULL, &comp)
		    == TRUE) {
			syslog(LOG_DEBUG, "cell[%s]: got arg%d 0x%016llx",
			       cell->attr.filename, cell->attr.has_args,
			       comp.int64);
			cell->attr.args_buf[cell->attr.has_args++] = comp.int64;
		}
	} else {
		syslog(LOG_DEBUG, "cell[%s]: needs no arg.",
		       cell->attr.filename);
	}

	/* 引数が揃っていなければ実行できない */
	bool_t is_executable;
	if ((cell->attr.num_args > 0)
	    && (cell->attr.has_args < cell->attr.num_args)) {
		is_executable = FALSE;
		syslog(LOG_DEBUG, "cell[%s]: not executable.",
		       cell->attr.filename);
	} else {
		is_executable = TRUE;
		syslog(LOG_DEBUG, "cell[%s]: executable.",
		       cell->attr.filename);
	}

	return is_executable;
}

static bool_t growth(struct cell *cell)
{
	syslog(LOG_DEBUG, "cell[%s]: starts growth.", cell->attr.filename);

	struct compound comp;
	char comp_str[MAX_COMPOUND_ELEMENTS * 3];

	/* コード化合物を一つ取得 */
	syslog(LOG_DEBUG, "%s: a", __FUNCTION__);
	if (sysenv_get_comp(cell, COMP_TYPE_CODE, NULL, &comp) == FALSE) {
		syslog(LOG_DEBUG, "%s: b", __FUNCTION__);
		syslog(LOG_DEBUG, "cell[%s]: missed getting a code.",
		       cell->attr.filename);
		return FALSE;
	}
	syslog(LOG_DEBUG, "%s: c", __FUNCTION__);
	syslog(LOG_DEBUG, "cell[%s]: got code %s.",
	       cell->attr.filename, comp_make_str(&comp, comp_str));

	syslog(LOG_DEBUG, "%s: d", __FUNCTION__);

	unsigned int i;

	/* 空きのあるコドンがあれば結合する */
	bool_t is_used = FALSE;
	for (i = 0; i < cell->attr.num_codns; i++) {
		if ((cell->codn_list[i].int64 == comp.int64)
		    && (cell->codn_list[i].is_buffered == FALSE)) {
			cell->codn_list[i].is_buffered = TRUE;
			is_used = TRUE;
			syslog(LOG_DEBUG,
			       "cell[%s]: [%s] was bonded to codon %d.",
			       cell->attr.filename, comp_str, i);
			break;
		}
	}

	/* 結合しなかった場合、環境へ排出する */
	if (is_used == FALSE) {
		syslog(LOG_DEBUG, "cell[%s]: [%s] was not bonded. (releasing)",
		       cell->attr.filename, comp_str);
		sysenv_put_comp(COMP_TYPE_CODE, NULL, &comp);
	}

	/* 細胞分裂可能か否かを判定 */
	for (i = 0; i < cell->attr.num_codns; i++) {
		if (cell->codn_list[i].is_buffered == FALSE) {
			syslog(LOG_DEBUG, "cell[%s]: not dividable.",
			       cell->attr.filename);
			return FALSE;
		}
	}
	syslog(LOG_DEBUG, "cell[%s]: dividable.", cell->attr.filename);
	return TRUE;
}

void central_dogma(struct cell *cell, struct cell *cell_new)
{
	unsigned int i;

	cell_new->attr.func_size = 0;
	for (i = 0; i < cell->attr.num_codns; i++)
		cell_new->attr.func_size += cell->codn_list[i].len;

	cell_new->func = mmap(
		NULL, cell_new->attr.func_size, PROT_EXEC | PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	ASSERT(cell_new->func != MAP_FAILED);

	unsigned char *new_func_p = (unsigned char *)cell_new->func;
	for (i = 0; i < cell->attr.num_codns; i++) {
		memcpy(new_func_p, cell->codn_list[i].byte,
		       cell->codn_list[i].len);
		cell->codn_list[i].is_buffered = FALSE;
		new_func_p += cell->codn_list[i].len;
	}

	syslog(LOG_DEBUG, "cell[%s,new]: central dogma was done.",
	       cell->attr.filename);
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

static char *_malloc_codn_list_str_buf(
	struct codon *codn_list, unsigned long long num_codns)
{
	unsigned long long i;
	size_t codn_list_str_buf_size = 0;
	for (i = 0; i < num_codns; i++)
		codn_list_str_buf_size += (3 * codn_list[i].len) + 1;
	return malloc(codn_list_str_buf_size);
}

static void _mutation_log(
	struct cell *cell, struct cell *cell_new, char *mutate_action)
{
	char *codn_list_str = _malloc_codn_list_str_buf(
		cell->codn_list, cell->attr.num_codns);
	ASSERT(codn_list_str != NULL);
	codn_list_make_str(
		cell->codn_list, cell->attr.num_codns, codn_list_str);

	char *codn_new_list_str = _malloc_codn_list_str_buf(
		cell_new->codn_list, cell_new->attr.num_codns);
	ASSERT(codn_new_list_str != NULL);
	codn_list_make_str(
		cell_new->codn_list, cell_new->attr.num_codns,
		codn_new_list_str);

	syslog(LOG_INFO, "cell[%s,new]: mutate(%s): %s -> %s",
	       cell->attr.filename, mutate_action,
	       codn_list_str, codn_new_list_str);

	free(codn_list_str);
	free(codn_new_list_str);
}

/* ランダムに選出した命令をidxの位置に追加する
 * (元々idxの位置にあった命令は追加した命令の下に続くようにする) */
static void _mutation_insert(
	struct cell *cell, struct cell *cell_new, unsigned int idx)
{
	cell_new->attr.num_codns += 1;

	cell_new->codn_list =
		malloc(sizeof(struct codon) * cell_new->attr.num_codns);
	ASSERT(cell_new->codn_list != NULL);

	unsigned int i;

	for (i = 0; i < idx; i++) {
		cell_new->codn_list[i].len = cell->codn_list[i].len;
		cell_new->codn_list[i].is_buffered = FALSE;
		cell_new->codn_list[i]._rsv = 0;
		cell_new->codn_list[i]._rsv2 = 0;
		cell_new->codn_list[i].int64 = cell->codn_list[i].int64;
	}

	struct codon mutated;
	sysenv_get_mutated_codon(&mutated);
	char codn_str[MAX_COMPOUND_ELEMENTS * 3];
	syslog(LOG_DEBUG, "cell[%s,new]: mutated codn(insert) is [%s](%d).",
	       cell->attr.filename, codn_make_str(&mutated, codn_str),
	       mutated.len);

	cell_new->codn_list[idx].len = mutated.len;
	cell_new->codn_list[idx].is_buffered = FALSE;
	cell_new->codn_list[idx]._rsv = 0;
	cell_new->codn_list[idx]._rsv2 = 0;
	cell_new->codn_list[idx].int64 = mutated.int64;

	for (i = idx; i < cell->attr.num_codns; i++) {
		cell_new->codn_list[i + 1].len = cell->codn_list[i].len;
		cell_new->codn_list[i + 1].is_buffered = FALSE;
		cell_new->codn_list[i + 1]._rsv = 0;
		cell_new->codn_list[i + 1]._rsv2 = 0;
		cell_new->codn_list[i + 1].int64 = cell->codn_list[i].int64;
	}

	_mutation_log(cell, cell_new, "insert");
}

/* idxの位置の命令をランダムに選出した命令へ変更する */
static void _mutation_modify(
	struct cell *cell, struct cell *cell_new, unsigned int idx)
{
	cell_new->codn_list =
		malloc(sizeof(struct codon) * cell_new->attr.num_codns);
	ASSERT(cell_new->codn_list != NULL);

	unsigned int i;

	for (i = 0; i < cell_new->attr.num_codns; i++) {
		cell_new->codn_list[i].len = cell->codn_list[i].len;
		cell_new->codn_list[i].is_buffered = FALSE;
		cell_new->codn_list[i]._rsv = 0;
		cell_new->codn_list[i]._rsv2 = 0;
		cell_new->codn_list[i].int64 = cell->codn_list[i].int64;
	}

	struct codon mutated;
	sysenv_get_mutated_codon(&mutated);
	char codn_str[MAX_COMPOUND_ELEMENTS * 3];
	syslog(LOG_DEBUG, "cell[%s,new]: mutated codn(modify) is [%s](%d).",
	       cell->attr.filename, codn_make_str(&mutated, codn_str),
	       mutated.len);

	cell_new->codn_list[idx].len = mutated.len;
	cell_new->codn_list[idx].int64 = mutated.int64;

	_mutation_log(cell, cell_new, "modify");
}

/* idxの位置の命令を削除する */
static void _mutation_remove(
	struct cell *cell, struct cell *cell_new, unsigned int idx)
{
	cell_new->attr.num_codns -= 1;

	cell_new->codn_list =
		malloc(sizeof(struct codon) * cell_new->attr.num_codns);
	ASSERT(cell_new->codn_list != NULL);

	unsigned int i;

	for (i = 0; i < idx; i++) {
		cell_new->codn_list[i].len = cell->codn_list[i].len;
		cell_new->codn_list[i].is_buffered = FALSE;
		cell_new->codn_list[i]._rsv = 0;
		cell_new->codn_list[i]._rsv2 = 0;
		cell_new->codn_list[i].int64 = cell->codn_list[i].int64;
	}

	char codn_str[MAX_COMPOUND_ELEMENTS * 3];
	syslog(LOG_DEBUG, "cell[%s,new]: remove codn is [%s](%d).",
	       cell->attr.filename,
	       codn_make_str(&cell->codn_list[idx], codn_str),
	       cell->codn_list[idx].len);

	for (i = idx; i < cell_new->attr.num_codns; i++) {
		cell_new->codn_list[i].len = cell->codn_list[i + 1].len;
		cell_new->codn_list[i].is_buffered = FALSE;
		cell_new->codn_list[i]._rsv = 0;
		cell_new->codn_list[i]._rsv2 = 0;
		cell_new->codn_list[i].int64 = cell->codn_list[i + 1].int64;
	}

	_mutation_log(cell, cell_new, "remove");
}

static void mutation(struct cell *cell, struct cell *cell_new)
{
	enum mutate_action {
		INSERT,
		MODIFY,
		REMOVE,
		MAX_MUTATE_ACTION
	};

	/* 方針: 末尾のret命令自身は触らない */

	if (cell_new->attr.num_codns >= 2) {
		syslog(LOG_DEBUG,
		       "cell[%s,new]: has code(s)(%lld) other than ret instruction.",
		       cell->attr.filename, cell_new->attr.num_codns);

		/* ret命令以外の命令が1つ以上ある
		 * (追加/変更/削除が可能) */
		unsigned int idx;
		switch (rand() % MAX_MUTATE_ACTION) {
		case INSERT:
			idx = rand() % cell_new->attr.num_codns;
			syslog(LOG_DEBUG, "cell[%s,new]: mutate action is insert."
			       " (idx=%d)", cell->attr.filename, idx);
			_mutation_insert(cell, cell_new, idx + 1);
			break;

		case MODIFY:
			if (cell_new->attr.num_codns > 2)
				idx = rand() % (cell_new->attr.num_codns - 1);
			else
				idx = 0;
			syslog(LOG_DEBUG, "cell[%s,new]: mutate action is modify."
			       " (idx=%d)", cell->attr.filename, idx);
			_mutation_modify(cell, cell_new, idx);
			break;

		case REMOVE:
			if (cell_new->attr.num_codns > 2)
				idx = rand() % (cell_new->attr.num_codns - 1);
			else
				idx = 0;
			syslog(LOG_DEBUG, "cell[%s,new]: mutate action is remove."
			       " (idx=%d)", cell->attr.filename, idx);
			_mutation_remove(cell, cell_new, idx);
			break;
		}
	} else {
		syslog(LOG_DEBUG,
		       "cell[%s,new]: has only ret instruction(num_codns=%lld).",
		       cell->attr.filename, cell_new->attr.num_codns);

		/* ret命令のみの状態
		 * (追加のみ可能) */

		/* ランダムに選出した命令をretの前に追加する */
		_mutation_insert(cell, cell_new, 0);
	}
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
	/* func_size はセントラルドグマの際に更新 */
	for (i = 0; i < CELL_MAX_ARGS; i++)
		cell_new.attr.args_buf[i] = 0;
	cell_new.attr.num_codns = cell->attr.num_codns;
	cell_new.attr.filename[0] = '\0';

	/* DNAを複製(ある確率で新細胞のDNAに突然変異を起こす) */
	bool_t do_mutate = (rand() % 100) < CELL_MUTATION_PROBABILITY;
	if (do_mutate) {
		/* 突然変異するなら突然変異しながらDNAを複製する */
		syslog(LOG_DEBUG, "cell[%s]: mutation.", cell->attr.filename);
		mutation(cell, &cell_new);
	} else {
		/* 突然変異しないならDNAの複製のみ行う */
		cell_new.codn_list = copy_codon_list(cell);
		syslog(LOG_DEBUG,
		       "cell[%s]: new cell's codons were created. (copy)",
		       cell->attr.filename);
	}

	/* 現細胞のDNAから生成したタンパク質リストを新細胞へ繋ぐ */
	central_dogma(cell, &cell_new);

	char *cell_str_buf = malloc(CELL_STR_BUF_SIZE);
	ASSERT(cell_str_buf != NULL);
	syslog(LOG_INFO,
	       "cell[%s,new]: a new cell was generated as follows.\n%s",
	       cell->attr.filename, cell_make_str(&cell_new, cell_str_buf));
	free(cell_str_buf);

	/* 新細胞を環境へ放出 */
	sysenv_put_cell(&cell_new);
	syslog(LOG_INFO, "cell[%s,%s]: %s was saved.", cell->attr.filename,
	       cell_new.attr.filename, cell_new.attr.filename);
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

	syslog(LOG_INFO, "cell[%s]: dead.", cell->attr.filename);
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
	syslog(LOG_DEBUG, "cell[%s]: load from file.", cell->attr.filename);

	/* ファイルを開く */
	FILE *load_fp = open_cell_file(cell, "rb");
	ASSERT(load_fp != NULL);

	size_t n;
	size_t read_bytes;

	/* 属性情報をロード */
	n = sizeof(struct cell_attributes) - MAX_FILENAME_LEN;
	read_bytes = fread_safe(&cell->attr, n, load_fp);
	ASSERT(read_bytes == n);
	syslog(LOG_DEBUG, "cell[%s]: attributes were loaded.",
	       cell->attr.filename);

	/* DNAをロード */
	n = sizeof(struct codon) * cell->attr.num_codns;
	cell->codn_list = malloc(n);
	ASSERT(cell->codn_list != NULL);
	read_bytes = fread_safe(cell->codn_list, n, load_fp);
	ASSERT(read_bytes == n);
	syslog(LOG_DEBUG, "cell[%s]: DNA were loaded.", cell->attr.filename);

	/* タンパク質をロード */
	cell->func = mmap(
		NULL, cell->attr.func_size, PROT_EXEC | PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	ASSERT(cell->func != MAP_FAILED);
	read_bytes = fread_safe(cell->func, cell->attr.func_size, load_fp);
	ASSERT(read_bytes == cell->attr.func_size);
	syslog(LOG_DEBUG, "cell[%s]: protein were loaded.",
	       cell->attr.filename);

	/* ファイルを閉じる */
	fclose(load_fp);

	syslog(LOG_DEBUG, "cell[%s]: load from file was finished.",
	       cell->attr.filename);
}

static void _save_file_to(struct cell *cell, bool_t do_free, FILE *save_fp)
{
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

void cell_save_to_file(struct cell *cell, bool_t do_free)
{
	syslog(LOG_DEBUG, "cell[%s]: save to file.", cell->attr.filename);

	/* ファイルを新規作成(存在する場合は上書き) */
	FILE *save_fp = open_cell_file(cell, "w+b");
	ASSERT(save_fp != NULL);

	_save_file_to(cell, do_free, save_fp);
}

void cell_save_as(struct cell *cell, bool_t do_free, char *path)
{
	/* ファイルを新規作成(存在する場合は上書き) */
	FILE *save_fp = fopen(path, "w+b");
	ASSERT(save_fp != NULL);

	_save_file_to(cell, do_free, save_fp);
}

void cell_remove_file(struct cell *cell)
{
	/* パスを作成 */
	char path[MAX_PATH_LEN + 1] = CELL_DIR_NAME;
	strncpy(&path[CELL_DIR_LEN], cell->attr.filename,
		(MAX_PATH_LEN + 1) - CELL_DIR_LEN);

	syslog(LOG_DEBUG, "cell[%s]: %s was removed.",
	       cell->attr.filename, path);

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
		if (cell.attr.fitness == CELL_MAX_FITNESS) {
			cell_save_as(&cell, FALSE, OUTPUT_FILENAME);
			syslog(LOG_DEBUG,
			       "cell[%s]: fitness is MAX_FITNESS(%d).",
			       cell.attr.filename, cell.attr.fitness);
			sysenv_exit();
			return;
		}
	}

	/* 成長 */
	bool_t is_divisible = growth(&cell);
	if (is_divisible == TRUE) {
		/* 増殖 */
		division(&cell);
	}

	/* 寿命を減らす */
	cell.attr.life_left--;
	syslog(LOG_DEBUG, "cell[%s]: life left has been decremented. (now=%d)",
	       cell.attr.filename, cell.attr.life_left);
	if (cell.attr.life_left == 0) {
		char *cell_str_buf = malloc(CELL_STR_BUF_SIZE);
		syslog(LOG_DEBUG, "cell[%s]: following cell life is zero.\n%s",
		       cell.attr.filename, cell_make_str(&cell, cell_str_buf));
		free(cell_str_buf);

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

char *codn_make_str(struct codon *codn, char *buf)
{
	unsigned char i;
	for (i = 0; i < codn->len; i++) {
		sprintf(&buf[i * 3], "%02x", codn->byte[i]);
		if (i < (codn->len - 1))
			buf[(i * 3) + 2] = ' ';
	}
	return buf;
}

char *codn_list_make_str(
	struct codon *codn_list, unsigned long long num_codns, char *buf)
{
	unsigned long long i;
	char *buf_p = buf;
	for (i = 0; i < num_codns; i++) {
		codn_make_str(&codn_list[i], buf_p);
		buf_p += (codn_list[i].len * 3) - 1;
		if (i < (num_codns - 1)) {
			*buf_p++ = ',';
			*buf_p++ = ' ';
		}
	}
	return buf;
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

char *cell_make_str(struct cell *cell, char *buf)
{
	unsigned int i, j;
	char *buf_p = buf;

	buf_p += sprintf(buf_p, "[Attributes]\n");
	buf_p += sprintf(buf_p, "- life_duration\t: %d\n",
			 cell->attr.life_duration);
	buf_p += sprintf(buf_p, "- life_left\t: %d\n", cell->attr.life_left);
	buf_p += sprintf(buf_p, "- fitness\t: %d\n", cell->attr.fitness);
	buf_p += sprintf(buf_p, "- num_args\t: %d\n", cell->attr.num_args);
	buf_p += sprintf(buf_p, "- has_args\t: %d\n", cell->attr.has_args);
	buf_p += sprintf(buf_p, "- has_retval\t: %d\n", cell->attr.has_retval);
	buf_p += sprintf(buf_p, "- func_size\t: %d\n", cell->attr.func_size);
	buf_p += sprintf(buf_p, "- args_buf\t:\n");
	for (i = 0; i < CELL_MAX_ARGS; i++) {
		buf_p += sprintf(buf_p, "  [%d] 0x%016llx\n",
				 i, cell->attr.args_buf[i]);
	}
	buf_p += sprintf(buf_p, "- num_codns\t: %lld\n", cell->attr.num_codns);

	buf_p += sprintf(buf_p, "\n");

	buf_p += sprintf(buf_p, "[DNA]\n");
	if (cell->codn_list == NULL)
		buf_p += sprintf(buf_p, "- codn_list\t: NULL\n");
	else {
		for (i = 0; i < cell->attr.num_codns; i++) {
			buf_p += sprintf(buf_p, "- codn_list[%d]\t:\n", i);
			buf_p += sprintf(buf_p, "  .len\t = %d\n",
					 cell->codn_list[i].len);
			buf_p += sprintf(buf_p, "  .is_buffered\t = %d\n",
			       cell->codn_list[i].is_buffered);
			buf_p += sprintf(buf_p, "  ._rsv\t = 0x%04x\n",
					 cell->codn_list[i]._rsv);
			buf_p += sprintf(buf_p, "  ._rsv2\t = 0x%08x\n",
			       cell->codn_list[i]._rsv2);
			buf_p += sprintf(buf_p, "  .int64\t = 0x%016llx\n",
			       cell->codn_list[i].int64);
			buf_p += sprintf(buf_p, "  .byte\t = 0x");
			for (j = 0; j < 8; j++) {
				buf_p += sprintf(buf_p, " %02x",
						 cell->codn_list[i].byte[j]);
			}
			buf_p += sprintf(buf_p, "\n");
		}
	}

	buf_p += sprintf(buf_p, "\n");

	buf_p += sprintf(buf_p, "[Protein]\n");
	if (cell->func == NULL)
		buf_p += sprintf(buf_p, "- func\t: NULL\n");
	else {
		buf_p += sprintf(buf_p, "- func\t: 0x\n");
		buf_p += sprintf(buf_p, " ");
		unsigned char *p = (unsigned char *)cell->func;
		for (i = 0; i < cell->attr.func_size; i++)
			buf_p += sprintf(buf_p, " %02x", p[i]);
	}
	buf_p += sprintf(buf_p, "\n");

	return buf;
}
