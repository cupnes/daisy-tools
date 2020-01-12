#pragma once

#include "compound.h"
#include "cell.h"

#define MAX_EXTCMD_LEN	100

#define EXTCMD_DIR	"bin/"
#define EXTCMD_NAME	"dsy-name"
#define EXTCMD_NAME_PATH	EXTCMD_DIR EXTCMD_NAME
#define EXTCMD_EVAL	"dsy-eval"
#define EXTCMD_EVAL_PATH	EXTCMD_DIR EXTCMD_EVAL

#define RUNNING_FILENAME	"running"
#define OUTPUT_FILENAME	"out.cell"

void sysenv_init(void);
void sysenv_do_cycle(void);
bool_t sysenv_get_comp(struct cell *cell,
		       enum COMP_TYPE type, char *name, struct compound *comp);
void sysenv_put_comp(enum COMP_TYPE type, char *name, struct compound *comp);
void sysenv_put_cell(struct cell *cell);
unsigned char sysenv_exec_and_eval(struct cell *cell);
void sysenv_get_mutated_codon(struct codon *codn);
bool_t sysenv_is_running(void);
void sysenv_exit(void);
