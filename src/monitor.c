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
#include <unistd.h>

#include <sys/wait.h>

#include "log.h"
#include "powerdown.h"

void monitor_handler(int signum)
{
	if (signum == SIGTSTP)
		pause();

	if (signum == SIGKILL || signum == SIGQUIT)
		exit(0);
}

int monitor_run(void)
{
	signal(SIGKILL, monitor_handler);
	signal(SIGTSTP, monitor_handler);
	signal(SIGQUIT, monitor_handler);

	for (;;) {
		waitpid(-1, NULL, WNOHANG);
		usleep(50);
	}

	return 0;
}
