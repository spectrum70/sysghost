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

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "process.h"
#include "fs.h"
#include "log.h"

#define PS_EXIT_US_INTERVAL	10000

int process_save_pid(const char *name, int pid)
{
	char fname[MAX_PATH] = {0};

	if (!fs_file_dir_exists(RUN_PID_PATH)) {
		if (fs_create_dir(RUN_PID_PATH, 0777)) {
			err("cannot create %s\n", RUN_PID_PATH);
			return -1;
		}
	}

	sprintf(fname, "%s%s", RUN_PID_PATH, name);

	fs_create_file_write_int(fname, (int)pid);

	return 0;
}

int process_running(pid_t pid)
{
	char dir_name[32];

	sprintf(dir_name, "%d", pid);

	return fs_file_dir_exists(dir_name);
}

int process_kill_by_name(const char *name)
{
	char fname[MAX_PATH] = {0};
	int pid;
	int timeout = 3000000 / PS_EXIT_US_INTERVAL;

	if (!name || *name == 0)
		return -1;

	sprintf(fname, "%s%s", RUN_PID_PATH, name);

	if (fs_file_read_int(fname, &pid) == 0) {

		if (kill((pid_t)pid, SIGKILL) == 0) {

			log_step("killing %s (%d) ...\n", name, pid);

			while (process_running(pid) && timeout--) {
				usleep(PS_EXIT_US_INTERVAL);
			}

			if (timeout < 0) {
				log_step_err("cannot kill process, skipping\n");
				return -1;
			}

			log_step("%s (%d) killed\n", name, pid);

			return 0;
		}
	}

	return -1;
}
