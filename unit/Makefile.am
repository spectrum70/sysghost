AUTOMAKE_OPTIONS = -Wno-portability

GIT_VERSION = $(shell git describe --abbrev=8 --dirty --always --tags)

bin_PROGRAMS = sysunit

sysunit_CPPFLAGS =-O0 -g -ggdb -I../include -DGIT_VERSION='"$(GIT_VERSION)"'
sysunit_SOURCES = main.c \
		   ../lib/log.c \
		   ../lib/fs.c \
		   ../src/process.c \
		   ../src/exec.c

sysunit_LDFLAGS = $(all_libraries)
sysunit_LDADD = ../lib/libcommon.a
