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

#include "log.h"

#include <stdarg.h>
#include <stdio.h>

static int loglevel = 6;
static int loglevel_max = 12;

static char vt100_bold[] = "\x1b[1m";
static char vt100_red[] = "\x1b[31;1m";
static char vt100_yellow[] = "\x1b[33;1m";
static char vt100_reset[] = "\x1b[0m";

void log(int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (level <= loglevel && level < loglevel_max) {
		switch(level) {
		case 0:
			printf(vt100_bold);
			break;
		case 1:
			printf(vt100_yellow);
			break;
		default:
			break;
		}

		vprintf(fmt, ap);

		printf(vt100_reset);
	}
	va_end(ap);
}

void err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	printf(vt100_red);
	printf("e  ");
	vprintf(fmt, ap);
	printf(vt100_reset);

	va_end(ap);
}

void log_step(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	printf("\x1b[93;1m * \x1b[0m");
	vprintf(fmt, ap);

	va_end(ap);
}

void log_step_err(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	printf("\x1b[31;1m * \x1b[0m");
	vprintf(fmt, ap);

	va_end(ap);
}
