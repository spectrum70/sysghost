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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/fs.h>
#include <linux/hdreg.h>

#include <sys/ioctl.h>

#define SYS_BLK		"/sys/block"
#define SYS_CLASS	"/sys/class/scsi_disk"
#define DEV_BASE	"/dev"
#define ISSPACE(c)	(((c) == ' ') || ((c) == '\n') || ((c) == '\t') || \
((c) == '\v') || ((c) == '\r') || ((c) == '\f'))

#define DISK_IS_IDE	0x00000001
#define DISK_IS_SATA	0x00000002
#define DISK_EXTFLUSH	0x00000004
#define DISK_REMOVABLE	0x00000008
#define DISK_MANAGED	0x00000010
#define DISK_FLUSHONLY	0x00000020

/* Used in flush_cache_ext(), compare with <linux/hdreg.h> */
#define IDBYTES		512
/* Bit 15 shall be zero, bit 14 shall be one, bit 13 flush cache ext */
#define MASK_EXT	0xE000
#define TEST_EXT	0x6000

/*
 * NOTE:
 * This code has been copied from sysvinit (C) halt.c
 * Needs to be verified and eventually adjusted.
 */

/*
 * Strip off trailing white spaces
 */
static char *strstrip(char *str)
{
	const size_t len = strlen(str);
	if (len) {
		char* end = str + len - 1;
		while ((end != str) && ISSPACE(*end))
			end--;
		*(end + 1) = '\0';     /* remove trailing white spaces */
	}
	return str;
}

/*
 * Open a sysfs file without getting a controlling tty and return
 * FILE* pointer.  Return 0 if the file didn't exist, or (FILE*)-1 if
 * something else went wrong.
 */
static FILE *hdopen(const char* const format, const char* const name)
{
	char buf[NAME_MAX+1];
	FILE *fp = (FILE*)-1;
	int fd, ret;

	ret = snprintf(buf, sizeof(buf), format, name);
	if ((ret >= (int)sizeof(buf)) || (ret < 0))
		goto error;             /* error */

	fd = open(buf, O_RDONLY|O_NOCTTY);
	if (fd < 0) {
		if (errno != ENOENT)
			goto error;     /* error */
		fp = (FILE*)0;
		goto error;             /* no entry `removable' */
	}

	fp = fdopen(fd, "r");
	if (fp == (FILE*)0)
		close(fd);              /* should not happen */
error:
	return fp;
}


/*
 * Check IDE/(S)ATA hard disk identity for
 * the FLUSH CACHE EXT bit set.
 */
static int flush_cache_ext(const char *device)
{
	#ifndef WIN_IDENTIFY
	#define WIN_IDENTIFY            0xEC
	#endif
	unsigned char args[4+IDBYTES];
	unsigned short *id = (unsigned short*)(&args[4]);
	char buf[NAME_MAX+1], *ptr;
	int fd = -1, ret = 0;
	FILE *fp;

	fp = hdopen(SYS_BLK "/%s/size", device);
	if (0 == (long)fp || -1 == (long)fp) {
		if (-1 == (long)fp)
			return -1;      /* error */
		goto out;               /* no entry `size' */
	}

	ptr = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if (ptr == (char*)0)
		goto out;               /* should not happen */

	ptr = strstrip(buf);
	if (*ptr == '\0')
		goto out;               /* should not happen */

	if ((size_t)atoll(buf) < (1<<28))
		goto out;               /* small disk */

	ret = snprintf(buf, sizeof(buf), DEV_BASE "/%s", device);
	if ((ret >= (int)sizeof(buf)) || (ret < 0))
		return -1;              /* error */

	if ((fd = open(buf, O_RDONLY|O_NONBLOCK)) < 0)
		goto out;

	memset(&args[0], 0, sizeof(args));
	args[0] = WIN_IDENTIFY;
	args[3] = 1;
	if (ioctl(fd, HDIO_DRIVE_CMD, &args))
		goto out;
	ret = snprintf(buf, sizeof(buf), DEV_BASE "/%s", device);
	if ((ret >= (int)sizeof(buf)) || (ret < 0))
		return -1;              /* error */

	if ((fd = open(buf, O_RDONLY|O_NONBLOCK)) < 0)
		goto out;

	memset(&args[0], 0, sizeof(args));
	args[0] = WIN_IDENTIFY;
	args[3] = 1;
	if (ioctl(fd, HDIO_DRIVE_CMD, &args))
		goto out;
#ifdef WORDS_BIGENDIAN
	id[83] = bswap_16(id[83]);
#endif
	if ((id[83] & MASK_EXT) == TEST_EXT)
		ret = 1;
out:
	if (fd >= 0)
		close(fd);
	return ret;
}

/*
 *      Find all disks through /sys/block.
 */
static char *list_disks(DIR* blk, unsigned int* flags)
{
	struct dirent *d;

	while ((d = readdir(blk))) {
		(*flags) = 0;
		if (d->d_name[1] == 'd' &&
			(d->d_name[0] == 'h' || d->d_name[0] == 's')) {
			char buf[NAME_MAX+1], lnk[NAME_MAX+1], *ptr;
			FILE *fp;
			int ret;

			fp = hdopen(SYS_BLK "/%s/removable", d->d_name);
			if (0 == (long)fp || -1 == (long)fp) {
				if (-1 == (long)fp)
					goto empty; /* error */
				continue;           /* no entry `removable' */
			}

			ret = getc(fp);
			fclose(fp);

			if (ret != '0')
				(*flags) |= DISK_REMOVABLE;

			if (d->d_name[0] == 'h') {
				if ((*flags) & DISK_REMOVABLE)
					continue;       /* not a hard disk */

				(*flags) |= DISK_IS_IDE;
				if ((ret = flush_cache_ext(d->d_name))) {
					if (ret < 0)
						goto empty;
					(*flags) |= DISK_EXTFLUSH;
				}
				/* old IDE disk not managed by kernel, out */
				break;
			}

			ret = snprintf(buf, sizeof(buf),
					SYS_BLK "/%s/device", d->d_name);
			if ((ret >= (int)sizeof(buf)) || (ret < 0))
				goto empty;             /* error */

			ret = readlink(buf, lnk, sizeof(lnk));
			if (ret >= (int)sizeof(lnk))
				goto empty;             /* error */
			if (ret < 0) {
				if (errno != ENOENT)
					goto empty;     /* error */
				continue;               /* no entry `device' */
			}
			lnk[ret] = '\0';

			ptr = basename(lnk);
			if (!ptr || !*ptr)
				continue;               /* should not happen */

			fp = hdopen(SYS_CLASS "/%s/manage_start_stop", ptr);
			if (0 == (long)fp || -1 == (long)fp) {
				if (-1 == (long)fp)
					goto empty;     /* error */
			} else {
				ret = getc(fp);
				fclose(fp);

				if (ret != '0') {
					(*flags) |= DISK_MANAGED;
					continue;
				}
			}

			fp = hdopen(SYS_BLK "/%s/device/vendor", d->d_name);
			if (0 == (long)fp || -1 == (long)fp) {
				if (-1 == (long)fp)
					goto empty; /* error */
				continue;           /* no entry */
			}

			ptr = fgets(buf, sizeof(buf), fp);
			fclose(fp);
			if (ptr == (char*)0)
				continue;           /* should not happen */

			ptr = strstrip(buf);
			if (*ptr == '\0')
				continue;           /* should not happen */

			if (strncmp(buf, "ATA", sizeof(buf)) == 0) {
				if ((*flags) & DISK_REMOVABLE)
					continue;   /* not a hard disk */

				(*flags) |= (DISK_IS_IDE|DISK_IS_SATA);
				if ((ret = flush_cache_ext(d->d_name))) {
					if (ret < 0)
						goto empty;
					(*flags) |= DISK_EXTFLUSH;
				}
				break; /* new SATA disk to shutdown, out here */
			}

			if (((*flags) & DISK_REMOVABLE) == 0)
				continue;  /* Seems to be a real SCSI disk */

			if ((ret = flush_cache_ext(d->d_name))) {
				if (ret < 0)
					goto empty;
				(*flags) |= DISK_EXTFLUSH;
			}
			break;  /* Removable disk like USB stick to shutdown */
		}
	}

	if (d == (struct dirent*)0)
		goto empty;

	return d->d_name;
	empty:
	return (char*)0;
}
/*
 *      Put an IDE/SCSI/SATA disk in standby mode.
 *      Code stolen from hdparm.c
 */
static int do_standby_disk(char *device, unsigned int flags)
{
	#ifndef WIN_STANDBYNOW1
	#define WIN_STANDBYNOW1         0xE0
	#endif
	#ifndef WIN_STANDBYNOW2
	#define WIN_STANDBYNOW2         0x94
	#endif
	#ifndef WIN_FLUSH_CACHE_EXT
	#define WIN_FLUSH_CACHE_EXT     0xEA
	#endif
	#ifndef WIN_FLUSH_CACHE
	#define WIN_FLUSH_CACHE         0xE7
	#endif
	unsigned char flush1[4] = {WIN_FLUSH_CACHE_EXT,0,0,0};
	unsigned char flush2[4] = {WIN_FLUSH_CACHE,0,0,0};
	unsigned char stdby1[4] = {WIN_STANDBYNOW1,0,0,0};
	unsigned char stdby2[4] = {WIN_STANDBYNOW2,0,0,0};
	char buf[NAME_MAX+1];
	int fd, ret;

	ret = snprintf(buf, sizeof(buf), DEV_BASE "/%s", device);
	if ((ret >= (int)sizeof(buf)) || (ret < 0))
		return -1;

	if ((fd = open(buf, O_RDWR|O_NONBLOCK)) < 0)
		return -1;

	switch (flags & DISK_EXTFLUSH) {
		case DISK_EXTFLUSH:
			if ((ret = ioctl(fd, HDIO_DRIVE_CMD, &flush1)) == 0)
				break;
		/* Else Fall through */
		/* Extend flush rejected, try standard flush */
		default:
			ret = ioctl(fd, HDIO_DRIVE_CMD, &flush2) &&
			ioctl(fd, BLKFLSBUF);
			break;
	}

	if ((flags & DISK_FLUSHONLY) == 0x0) {
		ret = ioctl(fd, HDIO_DRIVE_CMD, &stdby1) &&
		ioctl(fd, HDIO_DRIVE_CMD, &stdby2);
	}

	close(fd);

	if (ret)
		return -1;

	return 0;
}

/*
 *      List all disks and cause them to flush their buffers.
 */
int hdflush()
{
	unsigned int flags;
	char *disk;
	DIR *blk;

	if ((blk = opendir(SYS_BLK)) == (DIR*)0)
		return -1;

	while ((disk = list_disks(blk, &flags)))
		do_standby_disk(disk, (flags | DISK_FLUSHONLY));

	return closedir(blk);
}
