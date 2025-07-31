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
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include <linux/limits.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "log.h"

static const char ux_path[] = "/tmp/sg";

int ux_server_create(char *name)
{
	struct sockaddr_un address;
	char path[PATH_MAX] = {0};
	int fd;

	mkdir(ux_path, 0700);

	strcat(path, ux_path);
	strcat(path, "/");
	strcat(path, name);

	unlink(path);

	// Create the socket normally
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, path);

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
		return -1;

	if (bind(fd, (struct sockaddr*)&address, sizeof(address)))
		return -1;

	if (listen(fd, 10))
		return -1;

	return fd;
}

int ux_server_accept(int fd)
{
	fd_set rd_fds;
	int ret, cfd;
	struct timeval tout;

	tout.tv_sec = 0;
	tout.tv_usec = 1000;

	FD_ZERO(&rd_fds);
	FD_SET(fd, &rd_fds);

	ret = select(fd + 1, &rd_fds, 0, 0, &tout);
	if (ret > 0 && (FD_ISSET(fd, &rd_fds))) {
		cfd = accept(fd, NULL, NULL);
		if (cfd < 0)
			return -1;
		ret = fcntl(cfd, F_SETFL, fcntl(cfd, F_GETFL, 0) | O_NONBLOCK);
		if (ret == -1)
			return ret;

		return cfd;
	}

	return -1;
}

int ux_server_read_cmd(int fd, char *data)
{
	static int count;
	int i, ret;

	i = read(fd, data, 1024);
	if (i > 0) {
		count += i;
		if (strchr(data, '\n')) {
			ret = count;
			data[count] = 0;
			count = 0;
			close(fd);
			return ret;
		}
		data += i;
	}

	return -1;
}

/*
 * connect -> write -> close
 **/
int ux_client_write(char *path, char *command)
{
	struct sockaddr_un address;
	int fd, ret;

	address.sun_family = AF_UNIX;
        strcpy(address.sun_path, path);

	fd = socket(AF_UNIX, SOCK_STREAM, 0);

	ret = connect(fd, (struct sockaddr *)&address, sizeof(struct sockaddr));
 	if(ret == -1)
        	return ret;

	ret = send(fd, command, strlen(command), 0);
	if (ret <= 0) {
		close(fd);
		return ret;
	}

	return 0;
}
