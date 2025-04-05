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
#include <stdlib.h>

#include "exec.h"
#include "log.h"

int main(int argc, char **argv)
{
	if (getuid() != 0) {
		msg("please run the test as root.\n");
		exit(1);
	}

	log_step("test: exec_daemon (should fail)\n");
	exec_daemon("/usr/sbin/avahi-daemon -D -a2 -a3 -a4 -a5");

	log_step("test: exec_daemon (should succeed)\n");
	exec_daemon("/usr/sbin/avahi-daemon -D -s");

	log_step("test: completed\n");

	return 0;
}

