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

#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "fs.h"
#include "hdutils.h"
#include "log.h"

#define PATH_DEFAULT		"/sbin:/usr/sbin:/bin:/usr/bin"
#define WAIT_BETWEEN_SIGNALS	3

extern char **environ;

char *clean_env[] = {
	"HOME=/",
	"PATH=" PATH_DEFAULT,
	"TERM=dumb",
	"SHELL=/bin/sh",
	NULL,
};

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

void system_down(int reboot_system)
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
		msg("can't idle init: %s\r\n", strerror(errno));
		exit(1);
	}

	fprintf(stderr, "%s: sending TERM signal to all ...\r\n", __progname);

	/* Kill all processes. */
	kill(-1, SIGTERM);
	sleep(WAIT_BETWEEN_SIGNALS);

	fprintf(stderr, "%s: sending KILL signal to all ...\r\n", __progname);
	(void) kill(-1, SIGKILL);

	/* Kill init now */
	//kill(1, SIGKILL);

	/*
	 * To clarify it this is really blocking the shutdown
	 * and what
	 */
	spawn(1, "quotaoff", "-a", NULL);
	sync();
	spawn(0, "swapoff", "-a", NULL);
	spawn(0, "umount", "-a", NULL);

	/* Is this needed for a clear shutdown ? */
	//hdflush();

	sync();

	if (reboot_system) {
		fprintf(stderr, "%s: rebooting now ...\r\n", __progname);
		reboot(RB_AUTOBOOT);
	} else {
		fprintf(stderr, "sysghost; system halted.\r\n");
		reboot(RB_POWER_OFF);
	}
	/* WE NEVER GET HERE */
	exit(0);
}

