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

#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <stdlib.h>
#include <unistd.h>
#include <mntent.h>
#include <string.h>

#include "log.h"

#define UMOUNT_US_INTERVAL	100000

int umount_check_is_mounted(const char *dir)
{
	FILE *f;
	struct mntent *m;
	int mounted = 0;

	f = setmntent("/proc/mounts", "r");
	if (!f) {
		/* Cannot open mounts, return as mounted */
		return 1;
	}

	while ((m = getmntent(f)) != 0) {
		if (strcmp(dir, m->mnt_dir) == 0) {
			mounted = 1;
			break;
		}
	}

	endmntent(f);

	return mounted;
}

void umount_all(void)
{
	int timeout;

	log_step("unmounting efi partition ...\n");
	if (umount2("/boot", MNT_DETACH))
		log_step_err("unmount failed.\n");

	timeout = 3000000 / UMOUNT_US_INTERVAL;

	while (umount_check_is_mounted("/boot") && timeout--) {
		usleep(UMOUNT_US_INTERVAL);
	}

	if (timeout < 0) {
		log_step_err("unmount timeout.\n");
	}

	log_step("unmounting efi done\n");

	mkdir("/tmp/.old", 0755);

	mkdir("/tmp/proc", 0755);
	mkdir("/tmp/dev", 0755);
	mount("procfs", "/proc", "procfs", 0, "");
	mount("-", "/tmp/dev", "devtmpfs", 0, "");

	//if (syscall(SYS_pivot_root, "/tmp", "/tmp/.old")) {
	//	log_step_err("pivot_root failed.\n");
	//	return;
	//}

	chroot("/tmp");

	//if (umount2("/.old", MNT_DETACH))
	//	log_step_err("unmount failed.\n");

	//timeout = 3000000 / UMOUNT_US_INTERVAL;

	//while (umount_check_is_mounted("/") && timeout--) {
		sleep(1);
	//}

	//if (timeout < 0) {
	//	log_step_err("rootfs unmount timeout.\n");
	//}
}
