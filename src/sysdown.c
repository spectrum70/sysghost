/*
 * sysghost - a fast and simpie init
 *
 * C opyright (C) 2024 Kernelspace - Angelo Dureghello
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "fs.h"
#include "hdutils.h"
#include "log.h"

#define SD_VERSION	"0.90"

#define SYSDOWN_PID	"/var/run/sysdown.pid"
#define PATH_DEFAULT	"/sbin:/usr/sbin:/bin:/usr/bin"

#define WAIT_BETWEEN_SIGNALS	3

extern char *__progname;
extern char **environ;

enum options {
	opt_reboot = 1,
};

static enum options opts;

char *clean_env[] = {
	"HOME=/",
	"PATH=" PATH_DEFAULT,
	"TERM=dumb",
	"SHELL=/bin/sh",
	NULL,
};

void usage()
{
	msg("Usage: sysdown [option]\n");
	msg("Example: ./%s -r\n", __progname);
	msg("Options:\n");
	msg("  -h,  --help        this help\n");
	msg("  -r,  --reboot      reboot system\n");
	msg("  -V,  --version     program version\n");

	exit(-1);
}

void info()
{
	msg("sysdown (C) kernel-space.org - v." SD_VERSION "\n");
	exit(-1);
}

void cancel(int val)
{
	exit(-1);
}

int spawn(int silent, char *prog, ...)
{
	va_list ap;
	pid_t   pid, rc;
	int     i = 0;
	char    *argv[8];

	while ((pid = fork()) < 0 && i < 10) {
		perror("fork");
		sleep(3);
		i++;
	}

	if (pid < 0)
		return -1;

	if (pid > 0) {
		/* We are the parent here, waiting child to terminate. */
		while ((rc = wait(&i)) != pid) {
			if (rc < 0 && errno == ECHILD)
				break;
		}
		return (rc == pid) ? WEXITSTATUS(i) : -1;
	}

	/* Child has the job to shutdown. */
	if (silent)
		fclose(stderr);

	argv[0] = prog;
	va_start(ap, prog);
	for (i = 1; i < 7 && (argv[i] = va_arg(ap, char *)) != NULL; i++)
		;
	argv[i] = NULL;
	va_end(ap);

	if (chdir("/"))
		exit(1);

	/* Cleanup environment. */
	environ = clean_env;

	execvp(argv[0], argv);
	perror(argv[0]);
	exit(1);

	/*NOTREACHED*/
	return 0;
}

void system_down()
{
	int i;

	/* First close all stdin, stdout and stderr. */
	for(i = 2; i < 3; i++) {
		if (!isatty(i)) {
			/*
			 * Opening /dev/null with same number would cause
			 * messages to go there from now on.
			 */
			close(i);
			open("/dev/null", O_RDWR);
		}
	}

	/* Special kernel handled files as kmsg and simila other. */
	for(i = 3; i < 20; i++)
		close(i);

	/* TODO: mistery from sysvinit, to understand. */
	close(255);

	/* First idle init. */
	if (kill(1, SIGTSTP) < 0) {
		msg("%s: can't idle init: %s.\r\n",
		    __progname, strerror(errno));
		exit(1);
	}

	/* Kill all processes. */
	kill(-1, SIGTERM);
	sleep(WAIT_BETWEEN_SIGNALS);
	(void) kill(-1, SIGKILL);

	/* Kill init now */
	//kill(1, SIGKILL);

	//spawn(1, "quotaoff", "-a", NULL);

	sync();
	spawn(0, "swapoff", "-a", NULL);
	spawn(0, "umount", "-a", NULL);

	sync();
	hdflush();

	if (opts & opt_reboot) {
		sync();
		reboot(RB_AUTOBOOT);
		exit(0);
	}

	reboot(RB_POWER_OFF);
	exit(0);
}

int get_options(int argc, char **argv)
{
	int c;

	/* No options. */
	if (argc == 1)
		return 0;

	for (;;) {
		int option_index = 0;
		static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"version", no_argument, 0, 'V'},
			{"reboot", no_argument, 0, 'r'},
			{0, 0, 0, 0}
		};

                c = getopt_long(argc, argv, "hVr",
                                long_options, &option_index);

		if (c == -1) {
			/* End of options */
			return 0;
		}

		switch (c) {
		case 'h':
			usage();
			break;
		case 'V':
			info();
			break;
		case 'r':
			opts |= opt_reboot;
			break;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct sigaction sa;
	FILE *fp;
	int pid = 0;

	if (get_options(argc, argv))
		return -1;

	/*
	 * From sysvinit. Cannot really understand when/why an error would be
	 * thrown here, since from several g+s or u+s attempts could always
	 * be promoted (setuid) to 0. Keeping the check anyway.
	 */
	errno = 0;
	if (setuid(geteuid()) == -1) {
		msg("%s (%d): %s\n", __FILE__, __LINE__, strerror(errno));
		abort();
	}

	if (getuid() != 0) {
		msg("%s: only root can do that, exiting\n", __progname);
		exit(1);
	}

	/* Read pid of running sysdown from file. */
	if ((fp = fopen(SYSDOWN_PID, "r")) != NULL) {
		if (fscanf(fp, "%d", &pid) != 1)
			pid = 0;
		fclose(fp);
	}

	/* See if we are already running. */
	if (pid > 0 && kill(pid, 0) == 0) {
		msg("\r%s: already running.\r\n", __progname);
		exit(1);
	}

	/* Go to the root directory */
	if (chdir("/")) {
		msg("%s: chdir(/): %m\n", __progname);
		exit(1);
	}

	msg("asking for forcefsck\n");

	fs_touch("/forcefsck");

	msg("unlink and create new PID\n");
	/* Create a new PID file. */
	unlink(SYSDOWN_PID);
	umask(022);

	msg("catching signals ...\n");

	/* Catch some signals. */
	signal(SIGQUIT, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = cancel;
	sigaction(SIGINT, &sa, NULL);

	/* TODO: warn to all and library for it. */
	// wall()

	system_down();

	/* NOTREACHED */
	return 0;
}
