AUTOMAKE_OPTIONS = -Wno-portability

GIT_VERSION = $(shell git describe --abbrev=8 --dirty --always --tags)

bin_PROGRAMS = sysunit

sysunit_CPPFLAGS =-O0 -g -ggdb -I../include -DDEBUG  -DGIT_VERSION='"$(GIT_VERSION)"'
sysunit_SOURCES = main.c \
		   ../lib/log.c \
		   ../src/fs.c \
		   ../src/process.c \
		   ../src/exec.c
