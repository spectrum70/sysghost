AUTOMAKE_OPTIONS = -Wno-portability

GIT_VERSION = $(shell git describe --abbrev=8 --dirty --always --tags)

bin_PROGRAMS = sysghost sysdown

sysghost_CPPFLAGS = -I../include -DGIT_VERSION='"$(GIT_VERSION)"'
sysghost_SOURCES = main.c \
		../lib/log.c \
		launcher.c \
		mount.c \
		exec.c \
		process.c \
		logind.c \
		fs.c \
		umount.c \
		monitor.c

sysdown_CPPFLAGS = -I../include -DGIT_VERSION='"$(GIT_VERSION)"'
sysdown_SOURCES = ../lib/log.c \
		  sysdown.c

