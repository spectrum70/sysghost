AC_PREREQ([2.69])
AC_INIT([sysghost], [0.93(alpha)], [angelo@kernel-space.org], [sysghost], [http://www.kernel-space.org/])
AM_INIT_AUTOMAKE(subdir-objects)
AC_PROG_RANLIB

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_CHECK_TOOL(CC, gcc, gcc)

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "x$PKG_CONFIG" = "xno"; then
	AC_MSG_ERROR([You need to install pkg-config])
fi

AC_MSG_NOTICE(found $PKG_CONFIG)

CFLAGS="-O3 --pipe -Wall -Wno-builtin-declaration-mismatch"

# Substiture and create Makefile
AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile unit/Makefile])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h sys/ioctl.h termios.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([memset])

# udevd is the default
CFLAGS+=" -DUSE_UDEVD"

AC_OUTPUT
