AUTOMAKE_OPTIONS = -Wno-portability

GIT_VERSION = $(shell git describe --abbrev=12 --dirty --always)

bin_PROGRAMS = sysghost sysdown syslogin

sysghost_CPPFLAGS = -I../include -DGIT_VERSION='"$(GIT_VERSION)"'
sysghost_SOURCES = main.c \
		launcher.c \
		mount.c \
		exec.c \
		process.c \
		umount.c \
		monitor.c

sysghost_LDFLAGS = $(all_libraries)
sysghost_LDADD = ../lib/libcommon.a

sysdown_CPPFLAGS = -I../include -DGIT_VERSION='"$(GIT_VERSION)"'
sysdown_LDFLAGS = $(all_libraries)
sysdown_SOURCES = sysdown.c
sysdown_LDADD = ../lib/libcommon.a

syslogin_CPPFLAGS = -I../include -DGIT_VERSION='"$(GIT_VERSION)"'
syslogin_SOURCES = syslogin.c
syslogin_LDFLAGS = $(all_libraries) -lcrypt
