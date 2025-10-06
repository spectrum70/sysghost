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

#include <stdio.h>
#include <unistd.h>

#include "memory.h"

#define SIZE_KILO	(1024)
#define SIZE_MEGA       (SIZE_KILO * 1024)
#define SIZE_GIGA	(SIZE_MEGA * 1024)

void memory_get_total_size(char *h_size)
{
	int i;

	long long t_size = (long long)sysconf(_SC_PHYS_PAGES) *
				sysconf(_SC_PAGESIZE);
	long giga = t_size / SIZE_GIGA;
	long mega = (t_size % SIZE_GIGA) / SIZE_MEGA;

	i = snprintf(h_size, MAX_H_SIZE - 1, "%lu.%2lu GB", giga, mega);
	h_size[i] = 0;
}
