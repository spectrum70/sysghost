AUTOMAKE_OPTIONS = -Wno-portability

GIT_VERSION = $(shell git describe --abbrev=8 --dirty --always --tags)
AM_CXXFLAGS = -DGIT_VERSION='"$(GIT_VERSION)"'

bin_PROGRAMS = magic

magic_CPPFLAGS=-Iinclude
magic_SOURCES = src/main.c \
		src/log.c \
		src/launcher.c \
		src/mount.c \
		src/exec.c \
		src/powerdown.c \
		src/process.c \
		src/logind.c \
		src/fs.c \
		src/umount.c
