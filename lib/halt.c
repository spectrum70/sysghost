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
		sleep(5);
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

void close_all_streams(bool redirect_tty)
{
	int i;

	/* First close all stdin, stdout and stderr. */
	for(i = 0; i < 3; i++) {
		if (!redirect_tty)
			close(i);
		else if (!isatty(i)) {
			/*
			 * Opening /dev/null with same number would cause
			 * messages to go there from now on.
			 */
			close(i);
			/* Redirecting tty */
			open("/dev/null", O_RDWR);
		}
	}

	/* Special kernel handled files as kmsg and similar other. */
	for(i = 3; i < 20; i++)
		close(i);

	/* TODO: mistery from sysvinit, to understand. */
	close(255);
}

void finalize_reboot(int type)
{
	int i, ret;

	spawn(0, "umount", "-R", "/dev", NULL);

	for (i = 0; i < 3; i++) {
		/* Re-kill everybody if some process was not. */
		kill(-1, SIGKILL);
		ret = spawn(0, "umount", "-a", NULL);
		if (ret == 0)
			break;
		sync();
		sleep(1);
	}

	reboot(type);
}

void system_down(int reboot_system)
{
	close_all_streams(true);

	/* Kill all processes. */
	fprintf(stderr, "%s: sending TERM signal to all ...\r\n", __progname);
	kill(-1, SIGTERM);
	sleep(WAIT_BETWEEN_SIGNALS);

	fprintf(stderr, "%s: sending KILL signal to all ...\r\n", __progname);
	(void) kill(-1, SIGKILL);

	/* From now on, fprintf will not work. */

	sync();
	spawn(1, "quotaoff", "-a", NULL);
	sync();
	spawn(0, "swapoff", "-a", NULL);
	sync();

	/* Ask to init (pid 1) to unmount all and complete shutdown. */
	kill(1, reboot_system ? SIGUSR2 : SIGUSR1);

	/* Waiting for kill from pid 1, likey, or we exit. */
	sleep(0.5);

	exit(0);
}
