#include <stdio.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <sys/mman.h>

#include "cell.h"
#include "compound.h"
#include "sysenv.h"
#include "common.h"

#define ALL_CODE_LIST_LEN	4096
#define UPDATE_CODE_FILENAME	"update_code"

static struct compound all_code_list[ALL_CODE_LIST_LEN];
static unsigned int num_all_codes;

int dot_filter(const struct dirent *d_ent)
{
	/* '.'から始まるファイル名はFalse */
	if (d_ent->d_name[0] == '.')
		return 0;

	/* それ以外はTrue */
	return 1;
}

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

static void add_to_all_code_list(struct compound *comp)
{
	unsigned int i;

	for (i = 0; i < num_all_codes; i++) {
		if (comp->int64 == all_code_list[i].int64)
			return;
	}

	all_code_list[num_all_codes].int64 = comp->int64;
	all_code_list[num_all_codes].len = comp->len;
	num_all_codes++;
}

static void update_all_code_list(
	struct dirent **cell_namelist, int num_cell_files)
{
	int i;
	struct compound comp;

	for (i = 0; i < ALL_CODE_LIST_LEN; i++)
		all_code_list[i].len = 0;
	num_all_codes = 0;

	for (i = 0; i < num_cell_files; i++) {
		struct cell cell;
		strncpy(cell.attr.filename, cell_namelist[i]->d_name,
			MAX_FILENAME_LEN);
		cell_load_from_file(&cell);

		unsigned long long j;
		for (j = 0; j < cell.attr.num_codns; j++) {
			comp.len = cell.codn_list[j].len;
			comp.int64 = cell.codn_list[j].int64;
			add_to_all_code_list(&comp);
		}

		free(cell.codn_list);
		int _res = munmap(cell.func, cell.attr.func_size);
		ASSERT(_res == 0);
	}

	syslog(LOG_DEBUG, "%s: a", __FUNCTION__);

	char *dirname = comp_type2dir[COMP_TYPE_CODE];
	struct dirent **code_namelist;
	int num_code_files;
	num_code_files = scandir(dirname, &code_namelist, dot_filter, NULL);
	ASSERT(num_code_files >= 0);

	syslog(LOG_DEBUG, "%s: b", __FUNCTION__);

	for (i = 0; i < num_code_files; i++) {
		syslog(LOG_DEBUG, "%s: b1", __FUNCTION__);
		comp_load_from_file(dirname, code_namelist[i]->d_name, &comp);
		syslog(LOG_DEBUG, "%s: b2", __FUNCTION__);
		free(code_namelist[i]);
		syslog(LOG_DEBUG, "%s: b3", __FUNCTION__);
		add_to_all_code_list(&comp);
		syslog(LOG_DEBUG, "%s: b4", __FUNCTION__);
	}
	free(code_namelist);

	syslog(LOG_DEBUG, "%s: c", __FUNCTION__);
}

void sysenv_dump_all_code_list(void)
{
	unsigned int i;
	for (i = 0; i < num_all_codes; i++) {
		printf("[%04d] ", i);
		comp_print(&all_code_list[i]);
		printf("\n");
	}
}

void sysenv_init(void)
{
	srand((unsigned int)time(NULL));

	FILE *running_fp = fopen(RUNNING_FILENAME, "a+");
	fclose(running_fp);
}

void sysenv_do_cycle(void)
{
	static unsigned int num_cycle = 0;
	syslog(LOG_DEBUG, "sysenv: starts cycle %d.", num_cycle);

	/* 細胞ディレクトリ内のファイル一覧を取得 */
	struct dirent **cell_namelist;
	int num_cell_files =
		scandir(CELL_DIR_NAME, &cell_namelist, dot_filter, NULL);
	ASSERT(num_cell_files >= 0);
	ERROR_WITH(num_cell_files == 0, "No cell files.");

	/* 初回あるいは"update_code"ファイルが有れば全コードリストを更新 */
	FILE *update_code_fp = fopen(UPDATE_CODE_FILENAME, "r");
	if (update_code_fp != NULL) {
		fclose(update_code_fp);
		syslog(LOG_DEBUG, "sysenv: update all code list."
		       "(update_code file exists)");
		update_all_code_list(cell_namelist, num_cell_files);
		ASSERT(remove(UPDATE_CODE_FILENAME) == 0);
	} else if (num_cycle == 0) {
		syslog(LOG_DEBUG, "sysenv: update all code list."
		       " (this is the 1st cycle)");
		update_all_code_list(cell_namelist, num_cell_files);
	}

	syslog(LOG_DEBUG, "%s: a", __FUNCTION__);

	/* 各細胞の1周期を実施 */
	int i;
	for (i = 0; i < num_cell_files; i++) {
		/* TODO: マルチスレッド化 */
		syslog(LOG_DEBUG, "%s: b", __FUNCTION__);
		cell_do_cycle(cell_namelist[i]->d_name);
		syslog(LOG_DEBUG, "%s: c", __FUNCTION__);
		free(cell_namelist[i]);
	}
	free(cell_namelist);

	num_cycle++;
}

bool_t sysenv_get_comp(struct cell *cell,
		       enum COMP_TYPE type, char *name, struct compound *comp)
{
	ASSERT(type < MAX_COMP_TYPE);

	/* 自然淘汰 */
	int r = rand() % CELL_MAX_FITNESS;
	if (r >= cell->attr.fitness)
		return FALSE;

	char *dirname = comp_type2dir[type];

	syslog(LOG_DEBUG, "%s: a", __FUNCTION__);

	struct dirent **namelist;
	int num_files;
	num_files = scandir(dirname, &namelist, dot_filter, NULL);
	ASSERT(num_files >= 0);

	syslog(LOG_DEBUG, "%s: b", __FUNCTION__);

	if (num_files == 0) {
		close_namelist(namelist, num_files);
		return FALSE;
	}

	syslog(LOG_DEBUG, "%s: c", __FUNCTION__);

	if (name == NULL)
		name = namelist[rand() % num_files]->d_name;

	syslog(LOG_DEBUG, "%s: d", __FUNCTION__);

	comp_load_from_file(dirname, name, comp);
	comp_remove_file(dirname, name);

	syslog(LOG_DEBUG, "%s: e", __FUNCTION__);

	close_namelist(namelist, num_files);
	syslog(LOG_DEBUG, "%s: f", __FUNCTION__);
	return TRUE;
}

void sysenv_put_comp(
	enum COMP_TYPE type, char *specified_name, struct compound *comp)
{
	ASSERT(type < MAX_COMP_TYPE);

	char *dirname = comp_type2dir[type];

	char name[MAX_FILENAME_LEN];
	char *name_p = name;
	if (specified_name == NULL)
		create_filename(dirname, name_p, MAX_FILENAME_LEN);
	else
		name_p = specified_name;

	char comp_str[MAX_COMPOUND_ELEMENTS * 3];
	syslog(LOG_DEBUG, "sysenv: put [%s] to %s.",
	       comp_make_str(comp, comp_str), name_p);

	comp_save_to_file(dirname, name_p, comp);
}

void sysenv_put_cell(struct cell *cell)
{
	if (cell->attr.filename[0] == '\0') {
		create_filename(CELL_DIR_NAME, cell->attr.filename,
				MAX_FILENAME_LEN);
		syslog(LOG_DEBUG, "sysenv[%s]: was named \"%s\".",
		       cell->attr.filename, cell->attr.filename);
	}

	cell_save_to_file(cell, TRUE);
}

unsigned char sysenv_exec_and_eval(struct cell *cell)
{
	char cmd[MAX_EXTCMD_LEN];
	sprintf(cmd, "%s %s", EXTCMD_EVAL_PATH, cell->attr.filename);
	syslog(LOG_DEBUG, "sysenv[%s]: do %s", cell->attr.filename, cmd);

	int status = system(cmd);
	ASSERT(WIFEXITED(status));
	ASSERT(!WIFSIGNALED(status));
	ASSERT(!WIFSTOPPED(status));

	int exitstatus = WEXITSTATUS(status);
	ASSERT(exitstatus >= 0);
	ASSERT(exitstatus <= 100);

	return (unsigned char)exitstatus;
}

void sysenv_get_mutated_codon(struct codon *codn)
{
	unsigned int i = rand() % num_all_codes;
	codn->int64 = all_code_list[i].int64;
	codn->len = all_code_list[i].len;
}

bool_t sysenv_is_running(void)
{
	syslog(LOG_DEBUG, "%s: a", __FUNCTION__);

	bool_t is_running;

	FILE *running_fp = fopen(RUNNING_FILENAME, "r");
	if (running_fp != NULL) {
		fclose(running_fp);
		is_running = TRUE;
	} else
		is_running = FALSE;

	syslog(LOG_DEBUG, "%s: b %d", __FUNCTION__, is_running);

	return is_running;
}

void sysenv_exit(void)
{
	if (sysenv_is_running() == TRUE) {
		ASSERT(remove(RUNNING_FILENAME) == 0);
		syslog(LOG_DEBUG, "sysenv: exit. (running file was removed)");
	} else {
		syslog(LOG_DEBUG,
		       "sysenv: exit. (running file was already removed)");
	}
}
