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

#include <stdlib.h>
#include <unistd.h>
#include <mntent.h>
#include <string.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <errno.h>

#include "log.h"
#include "fs.h"

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

int umount_rootfs(void)
{
	int i;
	char bind_path[MAX_PATH];
	static const struct {
		const char *path;
		unsigned long flags;
	} transfer_table[] = {
		{ "/dev", MS_BIND | MS_REC },
		{ "/sys", MS_BIND | MS_REC },
		{ "/proc", MS_BIND | MS_REC  },
		{ "/run", MS_BIND },
	};

	/* We need to move to tmpfs now */
	if (fs_create_dir("/tmp/oldroot", 0755))
		return -1;
	if (fs_create_dir("/tmp/dev", 0755))
		return -1;
	if (fs_create_dir("/tmp/sys", 0755))
		return -1;
	if (fs_create_dir("/tmp/proc", 0755))
		return -1;
	if (fs_create_dir("/tmp/run", 0755))
		return -1;

	for (i = 0; i < (sizeof(transfer_table) / sizeof(transfer_table[0]));
		++i) {
		sprintf(bind_path, "/tmp%s", transfer_table[i].path);
		if (mount(transfer_table[i].path, bind_path, 0,
			  transfer_table[i].flags, 0)) {
			err("cannot bind-mount %s\n", transfer_table[i].path);
			return -1;
		}
	}

	/*
	 * See man pivot_root()
	 */
	chdir("/tmp");

	if (syscall(SYS_pivot_root, ".", ".")) {
		log_step_err("pivot_root failed.\n");
		return -1;
	}

	log_step("pivot root done.\n");

	sync();
	sleep(1);

	if (umount2(".", MNT_DETACH)) {
		log_step_err("rootfs umount failed.\n");
		return -1;
	}

	sync();

	log_step("old rootfs umount done\n");

	return 0;
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

	log_step("unmounting rootfs\n");

	if (umount_rootfs()) {
		log_step_err("failed to unmount rootfs\n");
		return;
	}

	sync();
	sync();

	sleep(1);
}
