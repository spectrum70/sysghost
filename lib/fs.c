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
#include <stdio.h>
#include <sys/stat.h>

#include "fs.h"
#include "log.h"

int fs_file_dir_exists(const char *path)
{
	struct stat st;

	if (stat(path, &st) == 0)
		return 1;

	return 0;
}

int fs_create_dir(const char *path, int mode)
{
	if (mkdir(path, 0777)) {
		err("cannot create dir %s\n", path);
		return -1;
	}

	return 0;
}

int fs_create_file_write_int(char *path, int num)
{
	FILE *f;

	f = fopen(path, "w");
	if (!f)
		return -1;

	fprintf(f, "%d\n", num);
	fclose(f);

	return 0;
}

int fs_create_file_write_str(char *path, char *str)
{
	FILE *f;

	f = fopen(path, "w");
	if (!f)
		return -1;

	fprintf(f, "%s\n", str);
	fclose(f);

	return 0;
}

int fs_file_read_int(const char *path, int *num)
{
	FILE *f;

	f = fopen(path, "r");
	if (!f)
		return -1;

	fscanf(f, "%d", num);
	fclose(f);

	return 0;
}

int fs_file_read_str(const char *path, char *str)
{
	FILE *f;
	size_t n = MAX_LINE, r;
	int rval = -1;

	f = fopen(path, "r");
	if (!f)
		return rval;

	if ((r = getline(&str, &n, f))) {
		str[r - 1] = 0;
		rval = 0;
	}

	fclose(f);

	return rval;
}

int fs_touch(const char *filename)
{
	int fd = open(filename, O_CREAT | S_IRUSR | S_IWUSR);

	if (fd == -1)
       		 return fd;

	close(fd);

	return 0;
}
