#include <time.h>
#include <unistd.h>

#include "sysenv.h"

#define WAIT_SEC	1

void init(void)
{
	srand((unsigned int)time(NULL));
}

int main(void)
{
	init();

	while (TRUE) {
		sysenv_do_cycle();
		sleep(WAIT_SEC);
	}
}
