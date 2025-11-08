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
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pthread.h>
#include <pwd.h>
#include <shadow.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#define VT100_COLOR_GREEN	"\x1b[92m"
#define VT100_COLOR_RESET	"\x1b[0m"

#define ERR_LOGIN		-1
#define ERR_NO_ROOT		-2
#define ERR_CANNOT_ENCRYPT	-3

#define PATH_USR_SERVICES	"/etc/sysghost/user"

static char is_tty;

// #define DEBUG_LOG

#ifdef DEBUG_LOG

#define PATH_LOG "/tmp/syslogin"

static FILE *flog;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int create_log_file()
{
	flog = fopen(PATH_LOG, "w");

	if (flog == 0)
		return -1;

	chmod(PATH_LOG, 0666);

	/* Turn off buffering, immediate write. */
	setvbuf(stdin, NULL, _IONBF, 0);
	fcntl(fileno(flog), F_SETFL,
	      fcntl(fileno(flog), F_GETFL) | O_DSYNC | O_RSYNC);

	return 0;
}

static void log(char *fmt, ...)
{
	va_list ap;

	pthread_mutex_lock(&mutex);

	if (flog) {
		int l = strlen(fmt);
		if (l > 0) {
			va_start(ap, fmt);
			vfprintf(flog, fmt, ap);
			if (fmt[l - 1] != '\n')
				fprintf(flog, "\n");
			va_end(ap);
			fflush(flog);
			sync();
		}
	}

	pthread_mutex_unlock(&mutex);
}
#else
static void log(char *fmt, ...)
{}
#endif /* DEBUG_LOG */

static const char *get_filename_ext(const char *filename)
{
	const char *dot = strrchr(filename, '.');

	if(!dot || dot == filename)
		return "";

	return dot + 1;
}

static void execute_script(char *name)
{
	int child_pid;

	child_pid = fork();
	if (child_pid == 0) {
		log("%s() executing script", __func__);
		/* Child, set as independent from parent */
		system(name);
		log("%s() script executed", __func__);
		exit(0);
	} else {
		int wstatus;

		log("%s() waiting fork script to be executed", __func__);

		do {
			waitpid(child_pid, &wstatus, 0);
		} while (!WIFEXITED(wstatus));

		log("%s() fork script executed", __func__);
	}
}

static void *thread_user(void *arg)
{
	DIR* dir;
	struct dirent *ent;
	struct stat states;
	char fname[PATH_MAX] = {0};
	const char *ext;

	log("%s() thread started", __func__);

	if (chdir(PATH_USR_SERVICES))
		return (void *)-1;

	dir = opendir(PATH_USR_SERVICES);

	while ((ent = readdir(dir)) != NULL) {
		stat(ent->d_name, &states);
		if (!strcmp(".", ent->d_name) || !strcmp("..", ent->d_name))
			continue;
		ext = get_filename_ext(ent->d_name);
		if (strcmp(ext, "sh"))
			continue;
		/* Executable ? */
		if(access(ent->d_name, X_OK))
			continue;
		/* Let's just go with system, no exec family, no replace. */
		strcat(fname, PATH_USR_SERVICES);
		strcat(fname, "/");
		strcat(fname, ent->d_name);
		execute_script(fname);
	}

	closedir(dir);

	log("%s() thread exiting", __func__);

	return (void *)0;
}

static void run_user_thread(void)
{
	/* Child */
	pthread_t thread;
	int ret;

	ret = pthread_create(&thread, NULL, thread_user, 0);
	if (ret != 0)
		printf("error creating user thread\n");

	pthread_join(thread, 0);

	log("%s() thread exited\n", __func__);
}

static void start_user_services(void)
{
	int child_pid, i_pid;;

	log("%s() entered", __func__);

	/* Dual fork, we want independant process to parse scripts. */
	child_pid = fork();
	if (child_pid == 0) {
		if (setsid() != -1) {
			i_pid = fork();
			if (i_pid == 0) {
				/*
				 * Thread joined,
				 * will not terminate until completed.
				* */
				run_user_thread();
				log("%s() run_user_thread exited", __func__);

				/* Terminate this subprocess */
				exit(0);
			}
		}
		/* Terminate. */
		exit(0);
	} else {
		log("%s() run_user_thread detached", __func__);
	}
}

static void ctrl_c_handler(int _)
{
	/* Invalidating exit */
	(void)_;
}

static void login_error(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "\n\n");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n\n");
	va_end(ap);
}

static int getch() {
	struct termios old_tio, new_tio;
	int ch;

	tcgetattr(STDIN_FILENO, &old_tio);
	new_tio = old_tio;
	new_tio.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

	return ch;
}

static void syslogin_get_input(char *in, int max_len, char pwd)
{
	int c = 0, len = 0;

	while (c != '\n' && len < max_len) {

		c = pwd ? getch() : getchar();

		if (c == 0x08 || c == 127) {
			if (len) {
				in[len] = 0;
				len--;
				if (pwd)
					printf("\b \b");
			}
		} else if (c >= 32 && c < 127) {
			in[len++] = c;

			if (pwd)
				printf("*");
		} else {
			/* Do something on special chars ? */
		}
	}
}

static void print_uname()
{
	struct utsname buffer;
	char *tty_name = ttyname(STDIN_FILENO);

	if (strstr(tty_name, "/tty") != 0)
		is_tty = 1;

	errno = 0;
	if (uname(&buffer) < 0) {
		perror("uname");
		exit(EXIT_FAILURE);
	}

	printf(VT100_COLOR_GREEN "%s %s on %s (%s)\n\n",
		buffer.sysname,
		buffer.release,
		buffer.machine,
		tty_name);
}

static int syslogin_prompt(void)
{
	char user[LOGIN_NAME_MAX + 1] = {0};
	char pwd[LOGIN_NAME_MAX + 1] = {0};
	char host[HOST_NAME_MAX + 1] = {0};
	char *encrypted;
	struct passwd* pwd_entry;
	struct spwd *spwd;

	signal(SIGINT, ctrl_c_handler);
	signal(SIGQUIT, ctrl_c_handler);
	signal(SIGTERM, ctrl_c_handler);

	print_uname();

	if (gethostname(host, HOST_NAME_MAX))
		return -1;

	printf(VT100_COLOR_GREEN "%s Login: ", host);

	syslogin_get_input(user, LOGIN_NAME_MAX, 0);

	/* Final check ? */
	/* ^[a-z][-a-z0-9]*$ */

	printf(VT100_COLOR_GREEN "Password: ");

	syslogin_get_input(pwd, LOGIN_NAME_MAX, 1);
	/* TODO: investigate OK in the output */
	printf("\n");

	pwd_entry = getpwnam(user);
	if (!pwd_entry) {
		login_error( "user '%s' doesn't exist", user);
		return ERR_LOGIN;
	}

	spwd = getspnam(user);
	if (spwd == NULL) {
		login_error("no permission to read shadow password file");
		return ERR_NO_ROOT;
	}

	/* If there is a shadow password record use it. */
	pwd_entry->pw_passwd = spwd->sp_pwdp;

	encrypted = crypt(pwd, pwd_entry->pw_passwd);
	memset(pwd, 0, sizeof(pwd));
	if (encrypted == NULL) {
		login_error("cannot encrypt, error %d\n", errno);
		return ERR_CANNOT_ENCRYPT;
	}

	gid_t groups[256];
	int ngroups = 255;

	if (strcmp(encrypted, pwd_entry->pw_passwd) == 0) {
		int ret;

		ret = getgrouplist(user, pwd_entry->pw_gid, groups, &ngroups);
		if (ret == -1)
			return ERR_LOGIN;
		setgid(pwd_entry->pw_gid);
		setgroups(ngroups, groups);
		/* Last, step down. */
		setuid(pwd_entry->pw_uid);
		/* Starting user services, tty only. */
		if (is_tty)
			start_user_services();
		/* GO */
		putenv("ZDOTDIR=/home/angelo");
		putenv("HOME=/home/angelo");
		execl(pwd_entry->pw_shell,
		      pwd_entry->pw_shell, "--login", NULL);
	} else {
		login_error("incorrect password\n");
		return ERR_LOGIN;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int attempts = 0;

#ifdef DEBUG_LOG
	create_log_file();
#endif

	while (syslogin_prompt() == ERR_LOGIN) {
		attempts++;
		sleep(1);
		if (attempts == 5) {
			login_error("10 seconds timeout ...\n");
			sleep(10);
		}
	}

	log("*** syslog exiting ***\n");

	return 0;
}
