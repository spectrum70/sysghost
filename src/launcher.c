/*
 * magic - a fast and simpie init
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <unistd.h>
#include <fcntl.h>

#include "log.h"
#include "mount.h"
#include "exec.h"
#include "process.h"

#define MAX_ENTRY	512

static int cores;

int launcher_get_cpus()
{
	cpu_set_t cs;

	sched_getaffinity(0, sizeof(cs), &cs);

	return CPU_COUNT_S(sizeof(cs), &cs);
}

void launcher_step_mount()
{
	log_step("mounting filesystems ...\n");
	mount_fstab();
	mount_rest();
}

void launcher_step_virtual_consoles()
{
	int i;
	char vc[][MAX_ENTRY] = {
		{"/sbin/agetty tty2 38400 linux &"},
		{"/sbin/agetty tty3 38400 linux &"},
		{"/sbin/agetty tty4 38400 linux &"},
		{0},
	};

	log_step("creating virtual consoles [2 to 4] ...\n");

	for (i = 0; *vc[i]; i++) {
		exec(vc[i]);
	}
}

void lanucher_step_run_list()
{
	int i;
	char list[][MAX_ENTRY] = {
		{"/bin/seatd -l silent -g seat >/dev/null &"},
		{"/usr/bin/wpa_supplicant -u -s -O /run/wpa_supplicant &"},
		{0},
	};

	for (i = 0; *list[i]; i++) {
		exec(list[i]);
	}
}

void launcher_init()
{
	pid_t pid;

	cores = launcher_get_cpus();

	log_step("available cpu cores: %d\n", cores);

	launcher_step_mount();

	log_step("launching udevd ...\n");

	if ((pid = fork()) == 0) {

		int outfd = open("/tmp/udev.tmp",
				 O_CREAT|O_WRONLY|O_TRUNC, 0644);
		if (!outfd)
		{
			log_step_err("failed to launch udevd, please reboot\n");
			return;
		}

		dup2(outfd, 1); // replace stdout
		close(outfd);

		execlp("/lib/systemd/systemd-udevd",
		       "/lib/systemd/systemd-udevd", NULL);
	} else {
		process_save_pid("udevd", (int)pid);

		launcher_step_virtual_consoles();
		lanucher_step_run_list();

		log_step("launching login ...\n");

		if ((pid = fork()) == 0) {
			execlp("/sbin/agetty", "/sbin/agetty",
				"--noclear", "tty1", "38400", "linux", NULL);
		}
	}

	for (;;) {
		sleep(10);
	}
}

