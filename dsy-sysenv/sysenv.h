#pragma once

#include "compound.h"
#include "cell.h"

#define MAX_EXTCMD_LEN	100

#define EXTCMD_DIR	"tools/"
#define EXTCMD_NAME	"dsy-name"
#define EXTCMD_NAME_PATH	EXTCMD_DIR EXTCMD_NAME

void sysenv_do_cycle(void);
bool_t sysenv_get_comp(enum COMP_TYPE type, char *name, struct compound *comp);
void sysenv_put_comp(enum COMP_TYPE type, char *name, struct compound *comp);
void sysenv_put_cell(char *name, struct cell *cell);
