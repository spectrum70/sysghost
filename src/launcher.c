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
 * Boston, MA 02110-1301, USA.lanucher_step_run_list(...)
 */

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/limits.h>

#include <sys/wait.h>

#include "cpu.h"
#include "fs.h"
#include "log.h"
#include "mount.h"
#include "exec.h"
#include "process.h"
#include "utils.h"

#define MAX_ENTRY	512

void launcher_step_mount()
{
	/* Steps and rrror check inside called functions. */
	mount_fstab();
	mount_rest();

	/* Check if we are already running ... */
	/* TODO: do this at very main.c init, checking for a mounted fs, */
	if (fs_check_running() != 0)
		exit(0);

	/* Now mark we are running. */
	fs_touch(FS_LOCKFILE);
}

void launcher_step_virtual_consoles()
{
	int i;
	char vc[][MAX_ENTRY] = {
		{"/sbin/agetty --skip-login --noissue "
			"--login-program /usr/local/bin/syslogin tty2 linux &"},
		{"/sbin/agetty --skip-login --noissue "
			"--login-program /usr/local/bin/syslogin tty3 linux &"},
		{"/sbin/agetty --skip-login --noissue "
			"--login-program /usr/local/bin/syslogin tty4 linux &"},
		{0},
	};

	for (i = 0; *vc[i]; i++) {
		exec_nowait(vc[i]);
	}
}

/*
 * Main idea was to hardcode the services here.
 */
void lanucher_step_run_services()
{
	int i;
	char list[][MAX_ENTRY] = {
		{"/usr/sbin/sshd"},
		{"/usr/bin/cupsd"},
		{"/usr/bin/avahi-daemon -D"},
		{0},
	};

	/* seatd behaves as an application, but is not terminating. */
	exec_nowait("/bin/seatd -l silent -g seat >/dev/null");

	for (i = 0; *list[i]; i++) {
		exec_daemon(list[i]);
	}
}

void launcher_run_dbus()
{
	int timeout = 500;
	/* Do not use const, since exec_daemon modify the cmd string. */
	char cmd_run[] = "/usr/bin/dbus-daemon --system";

	fs_create_dir("/run/dbus", 755);
	exec_daemon(cmd_run);

	/*
	 * We need to wait (5 seconds) the socket to be up, or
	 * other services as avahi will fail.
	 */
	log_step("waitinig for dbus socket ... ");

	for (; timeout; timeout--) {
		usleep(10000);
		if (fs_file_dir_exists("/run/dbus/system_bus_socket")) {
			log_step_success();
			return;
		}
	}
	/* Go on without dbus */
	log_step_err();
}

void launcher_init()
{
	pid_t pid;
#ifdef USE_UDEVD
	int status;
#endif

	/*
	 * Note: devtmpfs is already mounted at this stage, due to
	 * kernel config option DEVTMPFS_MOUNT.
	 */

#ifdef USE_UDEVD
	/*
	 * Very likely, a udevd is mandatory, because libinput in wayland
	 * seems to work only when a udevd is active, and seems there
	 * is not alternative to this.
	 */
	log_step("launching udevd ... ");

	/*
	 * Modern kernels uses devtmpfs that creates devices nodes when the
	 * kernel boots. Through devtmpfs, at initramfs stage some devices
	 * are probed and added to /dev.
	 * Some other, especially non built-in, are probed after real rootfs
	 * is mounted, but before udevd is launched. So the uevents for
	 * additions of these new devcices can't be seen from udevd
	 * For this reason, the following lines are needed after udevd start:
	 *   udevadm trigger --action=add --type=subsystems
	 *   udevadm trigger --action=add --type=devices
	 *   udevadm settle
	 * TODO: such lines are now in udevd.sh. See if it's the case to
         * execute those from here.
	 */
	if ((pid = fork()) == 0) {
		int outfd = open("/tmp/udev.tmp",
				 O_CREAT | O_WRONLY | O_TRUNC, 0644);
		if (!outfd) {
			log_step_err();
			return;
		}

		dup2(outfd, 1); // replace stdout
		dup2(outfd, 2);
		close(outfd);

		execlp("/lib/systemd/systemd-udevd",
		       "/lib/systemd/systemd-udevd",
		       "--daemon", ">/dev/null", NULL);
	}

	/* Wait to be launched */
	wait(&status);

	log_step_success();

	/*
	 * Keeping module probe as a first step after udevd init,
	 * becouse other services that are run later on may need
	 * some support from them.
	 */
	if (fs_file_dir_exists("/etc/sysghost"))
		exec_script("/etc/sysghost/udevd.sh");

#endif /* USE_UDEVD */

	log_step("available cpu cores: %d\n", cpu_get_cores_num());

	launcher_step_mount();
	launcher_run_dbus();

	if (fs_file_dir_exists("/etc/sysghost"))
		exec_script("/etc/sysghost/commands.sh");

	lanucher_step_run_services();

	launcher_step_virtual_consoles();

	if ((pid = fork()) == 0) {
		/* Child */
		execlp("/sbin/agetty", "/sbin/agetty",
			"--skip-login", "--noissue",
			"--login-program", "/usr/local/bin/syslogin",
			"--noclear", "tty1", "38400", "linux", NULL);
	}
}
