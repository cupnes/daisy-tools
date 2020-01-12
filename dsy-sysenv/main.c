#include <unistd.h>
#include <string.h>
#include <syslog.h>

#include "sysenv.h"

#define PROG_NAME	"dsy-sysenv"
#define DEFAULT_WAIT_SEC	1
#define DEFAULT_LOG_LEVEL	0

static void usage(void)
{
	fprintf(stderr, "Usage: %s [-h|CYCLE_WAIT_SEC]\n", PROG_NAME);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	unsigned int wait_sec;

	if (argc == 1) {
		wait_sec = DEFAULT_WAIT_SEC;
	} else if (argc == 2) {
		if (!strcmp(argv[1], "-h"))
			usage();
		wait_sec = (unsigned int)atoi(argv[1]);
	} else
		usage();

	openlog(PROG_NAME, LOG_CONS | LOG_PID, LOG_USER);
	syslog(LOG_DEBUG, "main: wait_sec is %d.", wait_sec);

	sysenv_init();

	while (sysenv_is_running() == TRUE) {
		syslog(LOG_DEBUG, "%s: a", __FUNCTION__);
		sysenv_do_cycle();
		syslog(LOG_DEBUG, "%s: a", __FUNCTION__);
		sleep(wait_sec);
		syslog(LOG_DEBUG, "%s: a", __FUNCTION__);
	}

	return EXIT_SUCCESS;
}
