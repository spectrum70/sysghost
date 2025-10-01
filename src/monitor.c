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

#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef SG_UNIX_SOCKET
#include <sys/mount.h>
#include <sys/stat.h>
#endif
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <sys/wait.h>

#include "log.h"
#include "halt.h"
#include "fs.h"

#ifdef SG_UNIX_SOCKET
#define TMPFS  "/mnt/tmpfs"

enum {
	ST_WAITING,
	ST_CONNECTED,
};
#endif


/* Unix socket communication */
#ifdef SG_UNIX_SOCKET
static void check_for_task(int fd)
{
	char buff[1024];
	static int status = ST_WAITING;
	static int cfd;

	switch (status) {
	default:
	case ST_WAITING:
		cfd = ux_server_accept(fd);
		if (cfd != -1)
			status = ST_CONNECTED;
		break;
	case ST_CONNECTED:
		if (ux_server_read_cmd(cfd, buff) > 0) {
			if (strncmp(buff, "shutdown", 8) == 0) {
				/* Close the server, waitng sysdown to exit */
				close(fd);
				/* Shutdown the system now */
				shutdown(0);
			} else if (strncmp(buff, "reboot", 6) == 0) {
				close(fd);
				shutdown(1);
			}
			status = ST_WAITING;
		}
		break;
	}
}

void *ux_server_run()
{
	int fd;

	fd = ux_server_create("init");
	if (fd == -1)
		return (void *)-1;

	for (;;) {
		check_for_task(fd);
		usleep(10000);
	}

	return 0;
}
#endif

static void monitor_shutdown(int type)
{
	/* Re-close all from here, as a safety check. */
	close_all_streams(false);

	/* Wait sysdon to exit */
	sync();
	sleep(1);

	/* Final steps, as umount and kill/close. */
	finalize_reboot(type);
}

static void monitor_handler(int signum)
{
	if (signum == SIGKILL) {
		/*
		 * Sysdown is not able to deliver this signal, seems due
		 * to lower privileges.
		 */
	}

	if (signum == SIGUSR1) {
		monitor_shutdown(RB_POWER_OFF);
	}

	if (signum == SIGUSR2) {
		monitor_shutdown(RB_AUTOBOOT);
	}

	if (signum == SIGTSTP)
		pause();
}

int monitor_run(void)
{
#ifdef SG_UNIX_SOCKET
	pthread_t t;

	pthread_create(&t, NULL, ux_server_run, 0);
#endif
	signal(SIGKILL, monitor_handler);
	signal(SIGTSTP, monitor_handler);
	signal(SIGQUIT, monitor_handler);
	signal(SIGUSR1, monitor_handler);
	signal(SIGUSR2, monitor_handler);

	for (;;) {
		waitpid(-1, NULL, WNOHANG);
		/* 50 msecs to remove zombies seems ok */
		usleep(50000);
	}

	return 0;
}
