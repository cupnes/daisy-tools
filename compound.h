#pragma once

#include "common.h"

#define COMP_CODE_DIR_NAME	"code/"
#define COMP_CODE_DIR_LEN	5
#define COMP_DATA_DIR_NAME	"data/"
#define COMP_DATA_DIR_LEN	5

#define MAX_COMPOUND_ELEMENTS	8

enum COMP_TYPE {
	COMP_TYPE_CODE,
	COMP_TYPE_DATA,
	MAX_COMP_TYPE
};

typedef unsigned long long comp_data_t;

extern char comp_type2dir[MAX_COMP_TYPE][MAX_DIR_LEN];

/* 化合物 */
struct __attribute__((packed)) compound {
	/* 長さ[バイト] */
	unsigned long long len;
	/* 次の共用体フィールドで指定する内容物の長さ。
	 * 共用体フィールドのサイズからも分かる通り、
	 * 指定できる最大サイズは8バイト。 */

	/* 内容物 */
	union {
		unsigned long long int64;
		unsigned char byte[8];
	};
};

FILE *comp_open_file(const char *dirname, const char *filename,
		     const char *mode);
void comp_load_from_file(
	const char *dirname, char *filename, struct compound *comp);
void comp_save_to_file(char *dirname, char *filename, struct compound *comp);
void comp_remove_file(const char *dirname, const char *filename);
void comp_dump(struct compound *comp);
void comp_print(struct compound *comp);
char *comp_make_str(struct compound *comp, char *buf);
