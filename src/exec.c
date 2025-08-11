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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "exec.h"
#include "log.h"
#include "process.h"

#define MAX_ARGS	32
#define MAX_ARG_LEN	32
#define MAX_PATH	512

enum {
	ex_wait_exit,
	ex_nowait,
	ex_daemon,
	ex_script,
};

static char *exec_trim_spaces(char *str)
{
	while (*str && *str == ' ')
		str++;

	return str;
}

static char *exec_fetch_arg(char *string, char *arg)
{
	char *p, *q;

	if (!string || !*string)
		return 0;

	q = exec_trim_spaces(string);
	if (!*q)
		return 0;

	p = strchr(q, ' ');
	if (!p)
		p = string + strlen(string);

	strncpy(arg, q, p - q);
	arg[p - q] = 0;

	return (*p == 0) ? 0 : ++p;
}

/*
 * Do not come with const char here, since we manipulate the string.
 */
static void exec_cmdline_to_argv(char *cmd_line, char **argv)
{
	char *q = cmd_line;
	int i = 1;

	/* Discard name with path, name only is retrieved before. */
	q = exec_fetch_arg(q, argv[i]);
	if (!q) {
		argv[i] = 0;
		return;
	}

	do {
		q = exec_fetch_arg(q, argv[i++]);
	} while (q && *q);

	/* Termination, should not be needed but just safe to do it. */
	argv[i] = 0;
}

static int exec_cmdline_get_path_name(const char *cmd_line,
				      char *path, char *name)
{
	const char *p = cmd_line;
	const char *endp;

	if (cmd_line == 0)
		return -1;

	while (*p != 0 && *p != ' ')
		p++;

	if (p == cmd_line)
		return -1;

	strncpy(path, cmd_line, p - cmd_line);

	endp = p;
	while (*p != '/' && p != path)
		p--;

	if (*p == '/')
		p++;

	strncpy(name, p, endp - p);

	return 0;
}

static int exec_build_argv(char **argv)
{
	int i;

	for (i = 0; i < MAX_ARGS; ++i) {
		argv[i] = malloc(MAX_ARG_LEN);
		if (argv[i] == NULL)
			return -ENOMEM;
		memset(argv[i], 0, MAX_ARG_LEN);
	}

	return 0;
}

static int __exec(char *cmd_line, int exec_type)
{
	char abs_name[MAX_PATH] = {0};
	char *argv[MAX_ARGS] = {0};
	pid_t pid;
	int ret;

	dbg("%s() cmd_line: %s\n", __func__, cmd_line);

	ret = exec_build_argv(argv);
	if (ret)
		return ret;

	ret = exec_cmdline_get_path_name(cmd_line, abs_name, argv[0]);
	if (ret)
		return ret;

	/*
	 * Some daemon as sshd wants argv[0] as an absolute path.
	 * As of now, let's set it for all, since it always work.
	 */
	strcpy(argv[0], abs_name);

	exec_cmdline_to_argv(cmd_line, argv);

	dbg("%s() %s: %s %s %s %s %s %s %s\n", __func__, abs_name,
		argv[0] ?: "null",
		argv[1] ?: "null",
		argv[2] ?: "null",
		argv[3] ?: "null",
		argv[4] ?: "null",
		argv[5] ?: "null",
		argv[6] ?: "null");

	if (exec_type != ex_daemon) {
		log_step("executing: %s ... ", argv[0]);
		if (exec_type == ex_script)
			msg("\n");
	} else
		log_step("daemon: starting %s ... ", argv[0]);

	if ((pid = fork()) == 0) {
		/*
		 * We need execv("/bin/ls", array);
		 * with first arg of array to be ls.
		 */
		execvp(abs_name, argv);

		dbg("(execvp errno: -%d %s)\n", errno, strerror(errno));
		exit(errno);
		/* Waitpid will catch the error. */
	} else {
		int status, rval;

		if (exec_type != ex_nowait) {
			rval = waitpid(pid, &status, 0);
		} else {
			rval = waitpid(pid, &status, WNOHANG);
		}

		dbg("rval %d\n", rval);
		dbg("WIFEXITED(status) %d\n", WIFEXITED(status));
		dbg("WEXITSTATUS(status) %d\n", WEXITSTATUS(status));

		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			process_save_pid(argv[0], (int)pid);
			if (exec_type != ex_script)
				log_step_success();
			ret = 0;
		} else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
			if (exec_type != ex_script)
				log_step_err();
			ret = -WEXITSTATUS(status);
		} else if (rval == 0) {
			/* Daemon is an app, WNOHANG case. */
			if (exec_type != ex_script)
				log_step_success();
			ret = 0;
		} else {
			if (exec_type != ex_script)
				log_step_err();
			ret = rval;
		}
	}

	return ret;
}

int exec_wait_exit(char *cmd_line)
{
	return __exec(cmd_line, ex_wait_exit);
}

int exec_nowait(char *cmd_line)
{
	return __exec(cmd_line, ex_nowait);
}

/* For daemons, we cannot wait termination. */
int exec_daemon(char *cmd_line)
{
	return __exec(cmd_line, ex_daemon);
}

int exec_script(char *name)
{
	return __exec(name, ex_script);
}
