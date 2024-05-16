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

#include <stdio.h>
#include <sys/stat.h>

#include "log.h"

int fs_dir_exists(const char *path)
{
	struct stat st;

	if (stat(path, &st) == 0)
		return 1;

	return 0;
}

int fs_create_dir(const char *path)
{
	return mkdir(path, 0777);
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
