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

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>

#include "log.h"
#include "exec.h"

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

int exec(char *cmd_line)
{
	pid_t pid;
	char *argv[MAX_ARGV] = {0};

	exec_cmdline_to_argv(cmd_line, argv);

	log_step("executing %s ...\n", argv[0]);
	if ((pid = fork()) == 0) {
		execvp(argv[0], argv);
	}

	return 0;
}
