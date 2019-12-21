#pragma once

#include <stdio.h>

#include "compound.h"
#include "common.h"

#define CELL_DIR_NAME	"cell/"
#define CELL_DIR_LEN	5

#define CELL_MAX_ARGS	4

struct __attribute__((packed)) cell_attributes {
	unsigned int life_duration;
	unsigned int life_left;

	unsigned char fitness;
	unsigned char num_args;
	unsigned char has_args;
	bool_t has_retval;
	unsigned int func_size;

	comp_data_t args_buf[CELL_MAX_ARGS];

	unsigned long long num_codns;
};

struct __attribute__((packed)) codon {
	unsigned char len;
	bool_t is_buffered;
	unsigned short _rsv;
	unsigned int _rsv2;
	union {
		unsigned long long int64;
		unsigned char byte[8];
	};
};

struct __attribute__((packed)) cell {
	/* 属性情報 */
	struct cell_attributes attr;

	/* DNA */
	struct codon *codn_list;

	/* タンパク質 */
	comp_data_t (*func)(comp_data_t, comp_data_t, comp_data_t, comp_data_t);
};

FILE *cell_open_file(const char *name, const char *mode);
void cell_load_from_file(char *name, struct cell *cell);
void cell_save_to_file(char *name, struct cell *cell, bool_t do_free);
void cell_remove_file(char *name);
void cell_do_cycle(char *filename);
void cell_dump(struct cell *cell);
