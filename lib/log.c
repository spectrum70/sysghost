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

#define esc		"\x1b["
#define g_red		esc "31m"
#define g_yellow	esc "33m"
#define g_gray		esc "37m"
#define g_blue		esc "94m"
#define g_fuxia		esc "95m"

#define g_bold		esc "1m"
#define g_reset		esc "0m"

#define g_bold_red	esc "1;" g_red

void msg(char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
}

void log(int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (level <= loglevel && level < loglevel_max) {
		switch(level) {
		case 0:
			printf(g_bold);
			break;
		case 1:
			printf(g_yellow);
			break;
		default:
			break;
		}

		vprintf(fmt, ap);

		printf(g_reset);
	}
	va_end(ap);
}

void err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	printf(g_red);
	printf("e  ");
	vprintf(fmt, ap);
	printf(g_reset);

	va_end(ap);
}

void log_step(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	printf(g_fuxia "● " g_reset);
	vprintf(fmt, ap);
	fflush(stdout);

	va_end(ap);
}

void log_skip(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	printf(g_gray "● " g_reset);
	vprintf(fmt, ap);

	va_end(ap);
}

void log_step_err()
{
	printf(g_bold_red "error" g_reset "\n");
}

void log_step_success()
{
	printf(g_fuxia "success" g_reset "\n");
}

void log_ghost_version(char *version)
{
	printf(g_fuxia "  .-.    \n"
	               " (o o)   " g_yellow "v.%s\n" g_fuxia
  		       " | O \\_  \n"
  		       "  `----' \n\n" g_reset, version);
}

