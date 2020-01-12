#pragma once

#include <stdio.h>

#include "compound.h"
#include "common.h"

#define CELL_MUTATION_PROBABILITY	10

#define CELL_MAX_MEMBER_NAME_LEN	100
#define CELL_MAX_MEMBER_VALUE_LEN	100

#define CELL_DIR_NAME	"cell/"
#define CELL_DIR_LEN	5

#define CELL_MAX_FITNESS	100
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

	char filename[MAX_FILENAME_LEN];
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
char *cell_make_str(struct cell *cell, bool_t is_verbose, char *buf);
