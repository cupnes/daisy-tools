#pragma once

#include <stdio.h>

#include "compound.h"
#include "common.h"

#define CELL_MUTATION_PROBABILITY	100

#define CELL_MAX_MEMBER_NAME_LEN	100
#define CELL_MAX_MEMBER_VALUE_LEN	100

#define CELL_DIR_NAME	"cell/"
#define CELL_DIR_LEN	5

#define CELL_MAX_FITNESS	100
#define CELL_MAX_ARGS	4

/* 細胞の属性情報 */
struct __attribute__((packed)) cell_attributes {
	/* 寿命[単位:サイクル数] */
	unsigned int life_duration;
	/* 余命[単位:サイクル数] */
	unsigned int life_left;
	/* 寿命と余命のメンバーがそれぞれある理由は、
	 * 細胞分裂時に突然変異しない場合は親の寿命と同じ寿命を子に設定するため。
	 * そして、1周期毎に余命フィールドをデクリメントしていく。
	 * 寿命フィールドは変更されることは無く、細胞分裂時に使われる。 */

	/* 適応度[単位:%] */
	unsigned char fitness;

	/* 引数の数 */
	unsigned char num_args;
	/* 取得済み引数の数 */
	unsigned char has_args;
	/* 戻り値の有無(1=有り/0=無し) */
	bool_t has_retval;

	/* 関数サイズ[単位:バイト] */
	unsigned int func_size;

	/* 引数バッファ */
	comp_data_t args_buf[CELL_MAX_ARGS];
	/* num_args・has_args・has_retval・args_bufは、
	 * 細胞の実行と評価にdsy-evalファイルを使用する新設計では
	 * 特に機能しないフィールド。 */

	/* DNAの長さ(コドンの数) */
	unsigned long long num_codns;

	/* 細胞ファイルのファイル名 */
	char filename[MAX_FILENAME_LEN];
};

/* コドン */
/* daisy-toolsでは「DNAを構成する要素」という意味で用いている。
 * 細胞を構成するタンパク質(機械語命令)一つ分が属性情報と共に
 * 一つのコドンに記録されている。
 * これを用いてセントラルドグマが成されることで細胞分裂する。 */
struct __attribute__((packed)) codon {
	/* 機械語命令の長さ[バイト] */
	unsigned char len;

	/* 既に取得済み(=1)か否か(=0) */
	bool_t is_buffered;
	/* 環境(codeディレクトリ)からコード化合物を取得した際、
	 * DNAを構成する一つ一つのコドンと比較し、
	 * 一致したら取り込む(is_bufferedフラグをセットする)。
	 * 一致しなかった場合は環境へ戻す。
	 * そして、DNAを構成する全コドンのis_bufferedがセットされたら
	 * 細胞分裂可能になる。 */

	/* 突然変異の挙動設定 */
	union {
		struct {
			/* このコドンの直前への挿入の不可フラグ */
			bool_t insp_dis: 1;
			/* このコドンの直後への挿入の不可フラグ */
			bool_t insn_dis: 1;
			/* このコドンの変更の不可フラグ */
			bool_t mod_dis: 1;
			/* このコドンの削除の不可フラグ */
			bool_t rem_dis: 1;
			/* 予約 */
			unsigned char _mf_rsv: 4;
		};
		unsigned char int8;
	} mutate_flg;

	/* 予約 */
	unsigned char _rsv;
	unsigned int _rsv2;

	/* タンパク質(機械語命令) */
	union {
		unsigned long long int64;
		unsigned char byte[8];
	};
};

/* 細胞 */
struct __attribute__((packed)) cell {
	/* 属性情報 */
	struct cell_attributes attr;

	/* DNA */
	struct codon *codn_list;

	/* タンパク質(機械語命令)の列へのポインタ */
	comp_data_t (*func)(comp_data_t, comp_data_t, comp_data_t, comp_data_t);
	/* このポインタの先の実体(機械語バイナリ列)は、細胞分裂時に親が
	 * 自身のDNAに従って蓄えたコード化合物を使って作られる。
	 * その際、ある確率でDNAが突然変異すると、子は親と異なるDNAを持つ。
	 * この様な流れであるため、突然変異した直後の細胞は
	 * 自信が持つ設計図(DNA)と、自身を構成する実体(タンパク質の列)が
	 * 異った状態になる。そして、この突然変異した細胞が細胞分裂することで、
	 * 突然変異した実体を持つ細胞が生まれる。 */
};

void cell_load_from_file(struct cell *cell);
void cell_save_to_file(struct cell *cell, bool_t do_free);
void cell_save_as(struct cell *cell, bool_t do_free, char *path);
void cell_remove_file(struct cell *cell);
void cell_do_cycle(char *filename);
void cell_exec(struct cell *cell, struct compound *prod);
char *codn_make_str(struct codon *codn, char *buf);
char *codn_list_make_str(
	struct codon *codn_list, unsigned long long num_codns, char *buf);
void cell_dump(struct cell *cell, bool_t is_verbose);
char *cell_make_json(struct cell *cell, bool_t is_verbose, char *buf);
