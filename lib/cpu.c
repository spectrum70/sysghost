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

/* getaffinity needs this */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <stdio.h>

#include <sys/utsname.h>

#include "cpu.h"

int cpu_get_cores_num()
{
	cpu_set_t cs;

	sched_getaffinity(0, sizeof(cs), &cs);

	return CPU_COUNT_S(sizeof(cs), &cs);
}

void cpu_get_architecture(char *arch)
{
	struct utsname os_info;
	int i;

	if (uname(&os_info) != 0) {
		i = snprintf(arch, MAX_CPU_ARCH - 1, "error");
	} else {
		i = snprintf(arch, MAX_CPU_ARCH - 1, "%s", os_info.machine);
	}

	arch[i] = 0;
}
