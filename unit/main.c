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

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "exec.h"
#include "log.h"
#include "socket.h"

static void *ux_client_thread(void *arg)
{
	sleep(2);

	ux_client_write("/tmp/sg/init", "hello\n");

	return 0;
}

static void exit_err()
{
	err("test failed, exiting.\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int ret;
	char buff[1024];
	pthread_t t;

	if (getuid() != 0) {
		msg("please run the test as root.\n");
		exit(1);
	}

	log_step("test: exec_daemon (should fail)\n");
	ret = exec_daemon("/usr/sbin/pippo");
	if (ret != 0)
		log_step("test: exec_daemon ok\n");
	else
		exit_err();

	system("killall avahi-daemon");
	sleep(1);
	log_step("test: exec_daemon (should succeed)\n");
	ret = exec_daemon("/usr/sbin/avahi-daemon -D -s");
	if (ret == 0)
		log_step("test: exec_daemon ok\n");
	else
		exit_err();

	log_step("test: create unix socket, waiting commands ...\n");
	int fd = ux_server_create("init");
	if (fd < 0)
		exit_err();

	pthread_create(&t, 0, ux_client_thread, 0);

	int cfd, rd = 0;
	for (;;) {
		cfd = ux_server_accept(fd);
		if (cfd >= 0) {
			for (;;) {
				if (ux_server_read_cmd(cfd, buff) > 0) {
					if (strcmp(buff, "hello\n") == 0) {
						rd = 1;
						break;
					}
				}
			}
		}
		if (rd)
			break;
	}

	log_step("test: unix socket, command received, test ok\n");

	return 0;
}

