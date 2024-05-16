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
#include <string.h>

#include "launcher.h"
#include "powerdown.h"
#include "log.h"
#include "version.h"

#define CMD_POWERDOWN	"powerdown"

int main(int argc, char **argv)
{
	if (strncmp(argv[0], CMD_POWERDOWN, strlen(CMD_POWERDOWN)) == 0)
		powerdown();

	log(0, " * " MAGIC_STR " init system v." VERSION " starting !\n");

	launcher_init();

	return 0;
}
