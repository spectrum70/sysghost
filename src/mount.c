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
#include "fs.h"

#define MAX_PATH	512
#define MAX_SWAP_LINE	1024
#define MOUNTS_PATH	"/run/sysghost/"

int mount_check_mounted(char *name, char *mnt_opts)
{
	struct mntent *ent;
	FILE *f;
	int ret = -1;

	f = setmntent("/proc/mounts", "r");
	if (!f) {
		err("setmntent");
		return ret;
	}

	while ((ent = getmntent(f)) != 0) {
		if (strncmp(name, ent->mnt_fsname, strlen(name)) != 0)
			continue;
		if (!mnt_opts) {
			ret = 0;
			break;
		}
		if (strncmp(mnt_opts, ent->mnt_opts, strlen(mnt_opts)) == 0) {
			ret = 0;
			break;
		}
	}

	endmntent(f);

	return ret;
}

int mount_check_swap_mounted(char *name)
{
	char line[MAX_SWAP_LINE];
	FILE *f;
	int ret = -1;

	f = fopen("/proc/swaps", "r");
	if (!f)
		return ret;

	while (fgets(line, MAX_SWAP_LINE - 1, f) != NULL) {
		if (strncmp(line, name, strlen(name)) == 0) {
			ret = 0;
			break;
		}
	}

	fclose(f);

	return ret;
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
		return -1;

	while ((p = strchr(opts, ','))) {
		*p = 0;

		if (*s == ' ' || *s == '\t')
			s++;

		mount_translate_opt(s, &val);

		s = p + 1;
	}

	return val;
}

int mount_rest(void)
{
	int ps_flags = MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_RELATIME;
	int ret;

	log_step("mounting pseudo filesystems ... ");

	ret = mkdir("/dev/pts", 0755);
	if (ret)
		goto err_step;
	ret = mkdir("/run/user", 0755);
	if (ret)
		goto err_step;
	ret = mkdir("/dev/mqueue", 0777);
	if (ret)
		goto err_step;

	ret = mount("devpts", "/dev/pts", "devpts", ps_flags, "");
	if (ret)
		goto err_step;
	ret = mount("debugfs", "/sys/kernel/debug", "debugfs", ps_flags, "");
	if (ret)
		goto err_step;
	ret = mount("securityfs", "/sys/kernel/security", "securityfs",
		    ps_flags, "");
	if (ret)
		goto err_step;
	ret = mount("tracefs", "/sys/kernel/tracing", "tracefs", ps_flags, "");
	if (ret)
		goto err_step;
	ret = mount("configfs", "/sys/kernel/config", "configfs", ps_flags, "");
	if (ret)
		goto err_step;
#if 0
	/*
	 * This step seems cannot be obrained from here, likely,
	 * get done from userspace.
	 */
	ret = mount("fusectl", "/sys/fs/fuse/connections", "fusectl",
		    ps_flags, "");
	if (ret)
		goto err_step;
#endif
	ret = mount("pstore", "/sys/fs/pstore", "pstore", ps_flags, "");
	if (ret)
		goto err_step;
	ret = mount("mqueue", "/dev/mqueue", "mqueue", ps_flags, "");
	if (ret)
		goto err_step;
	ret = mount("cgroup2", "/sys/fs/cgroup", "cgroup2", ps_flags, "");
	if (ret)
		goto err_step;

	log_step_success();

	log_step("mounting shared memory tmpfs ... ");

	ret = mkdir("/dev/shm", 0777);
	if (ret)
		goto err_step;
	ret = mount("tmpfs", "/dev/shm", "tmpfs", MS_NOSUID | MS_NODEV, "");
	if (ret)
		goto err_step;

	log_step_success();

	log_step("mounting tmp tmpfs ... ");

	ret = system("mount -t tmpfs "
		     "-o mode=1777,strictatime,nosuid,nodev,"
		     "size=16391740k,nr_inodes=1M tmpfs /tmp");
	if (ret)
		goto err_step;

	log_step_success();

	return 0;

err_step:
	log_step_err();

	return -1;
}

int mount_save_device(char *dev, char *name)
{
	char fname[MAX_PATH] = {0};

	if (!fs_dir_exists(MOUNTS_PATH)) {
		if (fs_create_dir(MOUNTS_PATH, 0777)) {
			err("cannot create /run/sysghost\n");
			return -1;
		}
	}

	sprintf(fname, MOUNTS_PATH "%s", name);

	return fs_create_file_write_str(fname, dev);
}

int mount_fstab(void)
{
	struct fstab *fs;
	char *dev_name;
	char spec[MAX_PATH] = {0};
	char name[MAX_PATH] = {0};

	if (setfsent() != 1) {
		err("could not open fstab\n");
		return -1;
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
			if (mount_check_swap_mounted(dev_name) == 0) {
				log_skip("swap already created, skipping\n");
				continue;
			} else {
				/*
				 * TODO: assuming default flags,
				 * priorities to be done later.
				 */
				log_step("activating swap ");

				if (swapon(dev_name, SWAP_FLAG_DISCARD) == -1) {
					log_step_err();
					continue;
				}

				log_step_success();

				if (mount_save_device(dev_name, "swap"))
					err("could not save swap dev\n");

				continue;
			}
		} else {
			/* Mount options can be different from initramfs ? */
			if (mount_check_mounted(dev_name, 0) == 0) {
				log_skip("%s already mounted, skipping\n",
				    name);
				continue;
			}
		}

		log_step("mounting %s to %s ", dev_name, fs->fs_file);

		if (mount(dev_name, fs->fs_file, fs->fs_vfstype,
		    mount_get_fstab_mnt_options(fs->fs_mntops), "") != 0) {
			log_step_err();
			continue;
		}

		log_step_success();
	}

	return 0;
}

