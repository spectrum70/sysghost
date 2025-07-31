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
#include "socket.h"

#define SD_VERSION	"0.90"

#define SYSDOWN_PID	"/var/run/sysdown.pid"

extern char *__progname;

enum options {
	opt_reboot = 1,
};

static enum options opts;

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

/*
 * sysdown should uses kill -1, so also sudo get killed and sudo sysdown would
 * not work. So, keeping sysghost as suid, as pid 1, by a signakl it can
 * perform the shudtown properly.
 */
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

	/* TODO: warn all */

	/* start sysdown by pid \1 here */
	if (opts & opt_reboot) {
	/* Communicate to init by unix socket */
		ux_client_write("/tmp/sg/init", "reboot\n");
	}

	ux_client_write("/tmp/sg/init", "shutdown\n");

	/* Exit asap, allow init to umount. */
	exit(0);
}
