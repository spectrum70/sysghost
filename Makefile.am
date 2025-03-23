AUTOMAKE_OPTIONS = -Wno-portability

GIT_VERSION = $(shell git describe --abbrev=8 --dirty --always --tags)

bin_PROGRAMS = sysghost sysdown

sysghost_CPPFLAGS = -Iinclude -DGIT_VERSION='"$(GIT_VERSION)"'
sysghost_SOURCES = src/main.c \
		src/log.c \
		src/launcher.c \
		src/mount.c \
		src/exec.c \
		src/process.c \
		src/logind.c \
		src/fs.c \
		src/umount.c \
		src/monitor.c

sysdown_CPPFLAGS = -Iinclude -DGIT_VERSION='"$(GIT_VERSION)"'
sysdown_SOURCES = src/sysdown.c
