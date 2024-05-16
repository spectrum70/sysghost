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


#include <fstab.h>
#include <sys/mount.h>
#include <sys/swap.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <mntent.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

#include "log.h"

#define MAX_PATH	512
#define MAX_SWAP_LINE	1024

int mount_check_mounted(char *name, char *mnt_opts)
{
	FILE *f;
	struct mntent *ent;

	f = setmntent("/proc/mounts", "r");
	if (!f) {
		err("setmntent");
		return 0;
	}

	while ((ent = getmntent(f)) != 0) {
		if (strncmp(name, ent->mnt_fsname, strlen(name)) != 0)
			continue;
		if (strncmp(mnt_opts, ent->mnt_opts, strlen(mnt_opts)) == 0)
			return 1;
	}

	endmntent(f);

	return 0;
}

int mount_check_swap_mounted(char *name)
{
	FILE *f;
	char line[MAX_SWAP_LINE];

	f = fopen("/proc/swaps", "r");
	if (!f)
		return 0;

	while (fgets(line, MAX_SWAP_LINE - 1, f) != NULL) {
		if (strncmp(line, name, strlen(name)) == 0)
			return 1;
	}

	fclose(f);

	return 0;
}

void mount_translate_opt(const char *opt, int *val)
{
	if (strncmp(opt, "rw", 2) == 0) {
		*val &= ~MS_RDONLY;
	} else if (strncmp(opt, "ro", 2) == 0) {
		*val |= MS_RDONLY;
	} else if (strncmp(opt, "relatime", 8) == 0) {
		*val |= MS_RELATIME;
	}
}

int mount_get_fstab_mnt_options(char *opts)
{
	int val = 0;
	char *s = opts, *p;

	if (!opts)
		return 0;

	while ((p = strchr(opts, ','))) {
		*p = 0;

		if (*s == ' ' || *s == '\t')
			s++;

		mount_translate_opt(s, &val);

		s = p + 1;
	}

	return val;
}

	void mount_rest(void)
{
	int ps_flags = MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_RELATIME;

	log_step("mounting pseudo filesystems ...\n");

	mkdir("/dev/pts", 0755);
	mkdir("/run/user", 0755);
	mkdir("/dev/mqueue", 0777);
	mount("devpts", "/dev/pts", "devpts", ps_flags, "");
	mount("debugfs", "/sys/kernel/debug", "debugfs", ps_flags, "");
	mount("securityfs", "/sys/kernel/security", "securityfs", ps_flags, "");
	mount("tracefs", "/sys/kernel/tracing", "tracefs", ps_flags, "");
	mount("configfs", "/sys/kernel/config", "configfs", ps_flags, "");
	mount("fusectl", "/sys/fs/fuse/connections", "fusectl", ps_flags, "");
	mount("pstore", "/sys/fs/pstore", "pstore", ps_flags, "");
	mount("mqueue", "/dev/mqueue", "mqueue", ps_flags, "");
	mount("cgroup2", "/sys/fs/cgroup", "cgroup2", ps_flags, "");

	log_step("mounting shared memory tmpfs ...\n");

	mkdir("/dev/shm", 0777);
	mount("tmpfs", "/dev/shm", "tmpfs", MS_NOSUID | MS_NODEV, "");

	log_step("mounting tmp tmpfs ...\n");

	system("mount -t tmpfs "
		"-o mode=1777,strictatime,nosuid,nodev,"
		"size=16391740k,nr_inodes=1M tmpfs /tmp");
}

void mount_fstab(void)
{
	struct fstab *fs;
	char *dev_name;
	char spec[MAX_PATH] = {0};
	char name[MAX_PATH] = {0};

	if (setfsent() != 1) {
		err("could not open fstab\n");
		return;
	}

	while((fs = getfsent())) {

		dev_name = fs->fs_spec;

		if (strncmp(fs->fs_spec, "UUID=", 5) == 0) {
			spec[0] = 0;
			strcat(spec, "/dev/disk/by-uuid/");
			strcat(spec, &fs->fs_spec[5]);

			dev_name = spec;

			if (realpath(spec, name) != 0)
				dev_name = name;
		}

		if (strncmp(fs->fs_vfstype, "swap", 4) == 0) {
			if (mount_check_swap_mounted(dev_name)) {
				log_step("* swap already created, skipping\n");
				continue;
			} else {
				/*
				 * TODO: assuming default flags,
				 * priorities to be done later.
				 */
				if (swapon(dev_name, SWAP_FLAG_DISCARD) == -1) {
					err("could not set swap on\n");
				}
				log_step("swap activated\n");
				continue;
			}
		} else {
			if (mount_check_mounted(name, fs->fs_mntops)) {
				log_step("%s already mounted, skipping\n",
				    name);
				continue;
			}
		}

		log_step("mounting %s to %s\n", dev_name, fs->fs_file);

		if (mount(dev_name, fs->fs_file, fs->fs_vfstype,
			  mount_get_fstab_mnt_options(fs->fs_mntops),
			  "") != 0) {
			log_step_err("could not mount %s\n", dev_name);
		}
	}
}

