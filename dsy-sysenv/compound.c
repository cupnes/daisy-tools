#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

#include "compound.h"
#include "common.h"

char comp_type2dir[MAX_COMP_TYPE][MAX_DIR_LEN] = {
	[COMP_TYPE_CODE] = COMP_CODE_DIR_NAME,
	[COMP_TYPE_DATA] = COMP_DATA_DIR_NAME
};

FILE *comp_open_file(const char *dirname, const char *filename,
		     const char *mode)
{
	/* パスを作成 */
	char path[MAX_PATH_LEN + 1];
	sprintf(path, "%s%s", dirname, filename);

	/* ファイルを開く */
	FILE *fp = fopen(path, mode);
	return fp;
}

void comp_load_from_file(
	const char *dirname, char *filename, struct compound *comp)
{
	syslog(LOG_DEBUG, "%s: a %s", __FUNCTION__, filename);

	FILE *load_fp = comp_open_file(dirname, filename, "rb");
	ASSERT(load_fp != NULL);

	syslog(LOG_DEBUG, "%s: b", __FUNCTION__);

	size_t read_bytes = fread_safe(comp, sizeof(struct compound), load_fp);
	ASSERT(read_bytes == sizeof(struct compound));

	syslog(LOG_DEBUG, "%s: c", __FUNCTION__);

	fclose(load_fp);

	syslog(LOG_DEBUG, "%s: d", __FUNCTION__);
}

void comp_save_to_file(char *dirname, char *filename, struct compound *comp)
{
	FILE *save_fp = comp_open_file(dirname, filename, "w+b");
	ASSERT(save_fp != NULL);

	size_t write_bytes = fwrite_safe(comp, sizeof(struct compound), save_fp);
	ASSERT(write_bytes == sizeof(struct compound));

	int fd = fileno(save_fp);
	ASSERT(fd != -1);
	int _res = fsync(fd);
	ASSERT(_res != -1);
	fclose(save_fp);
}

void comp_remove_file(const char *dir, const char *name)
{
	/* パスを作成 */
	char path[MAX_PATH_LEN + 1];
	sprintf(path, "%s%s", dir, name);

	/* 削除 */
	ASSERT(remove(path) == 0);
}

void comp_dump(struct compound *comp)
{
	printf("len\t: %lld\n", comp->len);
	printf("int64\t: 0x%016llx\n", comp->int64);
	printf("byte\t: 0x");
	unsigned char i;
	for (i = 0; i < sizeof(comp_data_t); i++)
		printf(" %02x", comp->byte[i]);
	printf("\n");
}

void comp_print(struct compound *comp)
{
	unsigned char i;
	for (i = 0; i < comp->len; i++) {
		printf("%02x", comp->byte[i]);
		if (i < (comp->len - 1))
			printf(" ");
	}
}

char *comp_make_str(struct compound *comp, char *buf)
{
	unsigned char i;
	for (i = 0; i < comp->len; i++) {
		sprintf(&buf[i * 3], "%02x", comp->byte[i]);
		if (i < (comp->len - 1))
			buf[(i * 3) + 2] = ' ';
	}
	return buf;
}
