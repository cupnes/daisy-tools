#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>

#define ERR_FMT		"at %s:%d %s()",	__FILE__, __LINE__, __FUNCTION__
#define ERR_FMT_MSG	"at %s:%d %s(): %s",	__FILE__, __LINE__, __FUNCTION__

int f_filter(const struct dirent *d_ent)
{
	/* 拡張子を抽出 */
	char *f_ext = strrchr(d_ent->d_name, '.');

	/* 拡張子が存在しなければFalse */
	if (f_ext == NULL)
		return 0;
	f_ext++;

	/* 拡張子が"bio"・"cmp"のどちらでも無ければFalse */
	if (strcasecmp(f_ext, "bio") && strcasecmp(f_ext, "cmp"))
		return 0;

	/* 通常ファイル以外ならFalse */
	struct stat sb;
	int result = stat(d_ent->d_name, &sb);
	if (result < 0)
		err(EXIT_FAILURE, ERR_FMT);
	if (!S_ISREG(sb.st_mode))
		return 0;

	/* それ以外はTrue */
	return 1;
}

int main(void)
{
	/* カレントディレクトリのファイル一覧を取得 */
	struct dirent **namelist;
	int num_files = scandir(".", &namelist, f_filter, NULL);
	if (num_files < 0)
		err(EXIT_FAILURE, ERR_FMT);
	if (num_files == 0)
		errx(EXIT_FAILURE, ERR_FMT_MSG, "No bio or cmp files.");

	int i;
	for (i = 0; i < num_files; i++) {
		printf("[%d] %s\n", i, namelist[i]->d_name);
		free(namelist[i]);
	}
	free(namelist);

	return 0;
}
