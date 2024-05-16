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

#include <unistd.h>
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "process.h"
#include "umount.h"
#include "log.h"

void powerdown(void)
{
	log_step("killing processes ...\n");

	process_kill_by_name("udevd");

	log_step("system power down, syncing\n");

	/* TODO:
	 * swapoff
	 */
	sync();

	log_step("unmounting all and switching off.\n");

	umount_all();
	setuid(0);

	sleep(1);
	reboot(RB_POWER_OFF);

	for (;;)
		sleep(1);
}
