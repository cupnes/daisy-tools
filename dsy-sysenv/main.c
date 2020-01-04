#include <time.h>
#include <unistd.h>

#include "sysenv.h"

#define WAIT_SEC	1
#define RUNNING_FILENAME	"running"

void init(void)
{
	srand((unsigned int)time(NULL));

	FILE *running_fp = fopen(RUNNING_FILENAME, "a+");
	fclose(running_fp);
}

int main(void)
{
	init();

	while (TRUE) {
		sysenv_do_cycle();

		FILE *running_fp = fopen(RUNNING_FILENAME, "r");
		if (running_fp == NULL)
			break;
		fclose(running_fp);

		sleep(WAIT_SEC);
	}
}
