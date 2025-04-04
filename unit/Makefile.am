AUTOMAKE_OPTIONS = -Wno-portability

GIT_VERSION = $(shell git describe --abbrev=8 --dirty --always --tags)

bin_PROGRAMS = sysunit

sysunit_CPPFLAGS = -I../include  -DGIT_VERSION='"$(GIT_VERSION)"'
sysunit_SOURCES = main.c \
		   ../lib/log.c \
		   ../src/exec.c
