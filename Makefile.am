AUTOMAKE_OPTIONS = -Wno-portability

GIT_VERSION = $(shell git describe --abbrev=8 --dirty --always --tags)
AM_CXXFLAGS = -DGIT_VERSION='"$(GIT_VERSION)"'

bin_PROGRAMS = sysghost sysdown

sysghost_CPPFLAGS = -Iinclude
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

sysdown_CPPFLAGS = -Iinclude
sysdown_SOURCES = src/sysdown.c
