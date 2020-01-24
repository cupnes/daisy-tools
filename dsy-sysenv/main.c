#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include "sysenv.h"

#define PROG_NAME	"dsy-sysenv"
#define DEFAULT_WAIT_USEC	1000000
#define DEFAULT_LOG_LEVEL	0

static void usage(void)
{
	fprintf(stderr, "Usage: %s [-h|CYCLE_WAIT_USEC]\n", PROG_NAME);
	exit(EXIT_FAILURE);
}

static void idle_usec(useconds_t usec)
{
	if (usleep(usec) == 0)
		return;

	switch (errno) {
	case EINTR:
		syslog(LOG_WARNING, "usleep: Interrupted by a signal. Continue.");
		break;

	case EINVAL:
		ERROR_WITH(TRUE, "usleep: usec is not smaller than 1000000.");
		break;
	}
}

int main(int argc, char *argv[])
{
	useconds_t wait_usec;

	if (argc == 1) {
		wait_usec = DEFAULT_WAIT_USEC;
	} else if (argc == 2) {
		if (!strcmp(argv[1], "-h"))
			usage();
		wait_usec = (useconds_t)atoi(argv[1]);
	} else
		usage();

	openlog(PROG_NAME, LOG_CONS | LOG_PID, LOG_USER);
	syslog(LOG_DEBUG, "main: wait_usec is %d.", wait_usec);

	sysenv_init();

	while (sysenv_is_running() == TRUE) {
		syslog(LOG_DEBUG, "%s: a", __FUNCTION__);
		sysenv_do_cycle();
		syslog(LOG_DEBUG, "%s: a", __FUNCTION__);
		idle_usec(wait_usec);
		syslog(LOG_DEBUG, "%s: a", __FUNCTION__);
	}

	return EXIT_SUCCESS;
}
