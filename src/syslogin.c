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

#include <bits/local_lim.h>

#include <crypt.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <shadow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>

#define VT100_COLOR_YELLOW	"\x1b[33m"
#define VT100_COLOR_RESET	"\x1b[0m"

#define ERR_LOGIN		-1
#define ERR_NO_ROOT		-2
#define ERR_CANNOT_ENCRYPT	-3

static void syslogin_get_input(char *in, int max_len)
{
	int c = 0, len = 0;

	while (c != '\n' && len < max_len) {
		c = getchar();
		if (c == 0x08) {
			if (len) {
				in[len] = 0;
				len--;
			}
		} else if (c >= 32 && c < 127) {
			in[len++] = c;
		} else {
			/* Do something on special chars ? */
		}
	}
}

static int syslogin_prompt(void)
{
	char user[LOGIN_NAME_MAX + 1] = {0};
	char pwd[LOGIN_NAME_MAX + 1] = {0};
	char host[HOST_NAME_MAX + 1] = {0};
	char *encrypted;
	struct passwd* pwd_entry;
	struct spwd *spwd;

	if (gethostname(host, HOST_NAME_MAX))
		return -1;

	printf(VT100_COLOR_YELLOW "%s Login: ", host);

	syslogin_get_input(user, LOGIN_NAME_MAX);

	/* Final check ? */
	/* ^[a-z][-a-z0-9]*$ */

	printf(VT100_COLOR_YELLOW "\nPassword: ");

	syslogin_get_input(pwd, LOGIN_NAME_MAX);

	pwd_entry = getpwnam(user);
	if (!pwd_entry) {
		printf( "User '%s' doesn't exist\n", user);
		return ERR_LOGIN;
	}

	spwd = getspnam(user);
	if (spwd == NULL) {
		printf("no permission to read shadow password file\n");
		return ERR_NO_ROOT;
	}

	/* If there is a shadow password record use it. */
	pwd_entry->pw_passwd = spwd->sp_pwdp;

	encrypted = crypt(pwd, pwd_entry->pw_passwd);
	memset(pwd, 0, sizeof(pwd));
	if (encrypted == NULL) {
		printf("cannot encrypt, error %d\n", errno);
		return ERR_CANNOT_ENCRYPT;
	}

	gid_t groups[256];
	int ngroups = 255;

	if (strcmp(encrypted, pwd_entry->pw_passwd) == 0) {
		int ret;

		printf("[%s] logged in\n", user);
		ret = getgrouplist(user, pwd_entry->pw_gid, groups, &ngroups);
		if (ret == -1)
			return ERR_LOGIN;
		setgid(pwd_entry->pw_gid);
		setgroups(ngroups, groups);
		/* Last, step down. */
		setuid(pwd_entry->pw_uid);
		/* GO */
		putenv("ZDOTDIR=/home/angelo");
		putenv("HOME=/home/angelo");
		execl(pwd_entry->pw_shell, pwd_entry->pw_shell, "--login", NULL);
	} else {
		printf("Incorrect password\n");
		return ERR_LOGIN;
	}

	return 0;
}

int main(int argc, char **argv)
{
	while (syslogin_prompt() == ERR_LOGIN)
		sleep(1);

	return 0;
}
