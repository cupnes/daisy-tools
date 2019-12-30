#include <stdio.h>
#include <string.h>

#include "cell.h"
#include "compound.h"
#include "sysenv.h"
#include "common.h"

static void create_filename(const char *dirname, char *name, int name_len)
{
	char cmd[MAX_EXTCMD_LEN];
	sprintf(cmd, "%s %s", EXTCMD_NAME_PATH, dirname);

	bool_t is_named = FALSE;
	while (is_named == FALSE) {
		FILE *pf = popen(cmd, "r");
		ASSERT(pf != NULL);

		char _tmp[MAX_FILENAME_LEN];
		char *_res = fgets(_tmp, MAX_FILENAME_LEN, pf);
		ASSERT(_res != NULL);

		char *_token = strtok(_tmp, "\r\n\0");
		ASSERT(_token != NULL);

		strncpy(name, _token, name_len);

		pclose(pf);

		FILE *_fp = comp_open_file(dirname, name, "rb");
		if (_fp == NULL)
			is_named = TRUE;
		else
			fclose(_fp);
	}
}

int dot_filter(const struct dirent *d_ent)
{
	/* '.'から始まるファイル名はFalse */
	if (d_ent->d_name[0] == '.')
		return 0;

	/* それ以外はTrue */
	return 1;
}

void sysenv_do_cycle(void)
{
	/* 細胞ディレクトリ内のファイル一覧を取得 */
	struct dirent **cell_namelist;
	int num_cell_files =
		scandir(CELL_DIR_NAME, &cell_namelist, dot_filter, NULL);
	ASSERT(num_cell_files >= 0);
	ERROR_WITH(num_cell_files == 0, "No cell files.");

	/* 各細胞の1周期を実施 */
	int i;
	for (i = 0; i < num_cell_files; i++) {
		/* TODO: マルチスレッド化 */
		cell_do_cycle(cell_namelist[i]->d_name);
		free(cell_namelist[i]);
	}
	free(cell_namelist);
}

bool_t sysenv_get_comp(enum COMP_TYPE type, char *name, struct compound *comp)
{
	ASSERT(type < MAX_COMP_TYPE);

	char *dirname = comp_type2dir[type];

	struct dirent **namelist;
	int num_files;
	num_files = scandir(dirname, &namelist, dot_filter, NULL);
	ASSERT(num_files >= 0);

	if (num_files == 0) {
		close_namelist(namelist, num_files);
		return FALSE;
	}

	if (name == NULL)
		name = namelist[rand() % num_files]->d_name;

	comp_load_from_file(dirname, name, comp);
	comp_remove_file(dirname, name);

	close_namelist(namelist, num_files);
	return TRUE;
}

void sysenv_put_comp(enum COMP_TYPE type, char *name, struct compound *comp)
{
	ASSERT(type < MAX_COMP_TYPE);

	char *dirname = comp_type2dir[type];

	char _name[MAX_FILENAME_LEN];
	if (name == NULL) {
		create_filename(dirname, _name, MAX_FILENAME_LEN);
		name = _name;
	}

	comp_save_to_file(dirname, name, comp);
}

void sysenv_put_cell(struct cell *cell)
{
	if (cell->attr.filename[0] == '\0') {
		create_filename(CELL_DIR_NAME, cell->attr.filename,
				MAX_FILENAME_LEN);
	}

	cell_save_to_file(cell, TRUE);
}

void sysenv_exec_and_eval(struct cell *cell)
{
	char cmd[MAX_EXTCMD_LEN];
	sprintf(cmd, "%s %s", EXTCMD_EVAL_PATH, cell->attr.filename);

	int status = system(cmd);
	ASSERT(WIFEXITED(status));
	ASSERT(!WIFSIGNALED(status));
	ASSERT(!WIFSTOPPED(status));

	int exitstatus = WEXITSTATUS(status);
	ASSERT(exitstatus >= 0);
	ASSERT(exitstatus <= 100);

	cell->attr.fitness = (unsigned char)exitstatus;
}
