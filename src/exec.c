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

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/wait.h>

#include "log.h"
#include "exec.h"
#include "process.h"

#define MAX_ARGV	128

char *exec_trim_spaces(char *cmd)
{
	while (*cmd && *cmd == ' ')
		cmd++;

	return cmd;
}

void exec_cmdline_to_argv(char *cmd_line, char **argv)
{
	char *q = cmd_line;
	int i = 0;

	do {
		q = exec_trim_spaces(q);
		argv[i++] = q;

		while (*q) {
			if (*q == ' ') {
				*q++ = 0;
				break;
			}
			q++;
		}

	} while (*q);

}

const char *exec_get_name(const char *name)
{
	const char *p = name, *rval = 0;

	while (*p) {
		if (*p++ == '/')
			rval = p;
	}

	return rval;
}

int __exec(char *cmd_line, int wait)
{
	pid_t pid;
	char *argv[MAX_ARGV] = {0};

	exec_cmdline_to_argv(cmd_line, argv);

	if (wait)
		log_step("daemon: %s ... ", argv[0]);
	else
		log_step("executing: %s ...\n", argv[0]);

	if ((pid = fork()) == 0) {
		execvp(argv[0], argv);
	} else {
		int status;

		process_save_pid(exec_get_name(argv[0]), (int)pid);
		/* Avoid zombies, always wait termination */
		if (wait) {
			waitpid(pid, &status, 0);
			log_step_success();
		}
	}

	/* We can't succeed here, executing .sh can exec
	 * and trace messages from other subtask (inside .sh) */

	return 0;
}

int exec(char *cmd_line)
{
	return __exec(cmd_line, 1);
}

/* For daemons, we cannot wait termination. */
int exec_daemon(char *cmd_line)
{
	return __exec(cmd_line, 0);
}
