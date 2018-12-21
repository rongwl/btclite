#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
define(_CLIENT_VERSION_MAJOR, 0)
define(_CLIENT_VERSION_MINOR, 1)
define(_CLIENT_VERSION_REVISION, 0)
AC_INIT([Btclite], [_CLIENT_VERSION_MAJOR._CLIENT_VERSION_MINOR._CLIENT_VERSION_REVISION], [https://github.com/rongwl/btclite/issues])
AC_CONFIG_SRCDIR([src/validation.cpp])
AC_CONFIG_HEADERS([src/config/btclite-config.h])
AC_CONFIG_MACRO_DIR([m4])

BTCLITE_DAEMON_NAME=btclited

AH_TOP([#ifndef BTCLITE_CONFIG_H])
AH_TOP([#define BTCLITE_CONFIG_H])
AH_BOTTOM([#endif // BTCLITE_CONFIG_H])

AM_INIT_AUTOMAKE([subdir-objects foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

dnl Require C++11 compiler (no GNU extensions)
AX_CXX_COMPILE_STDCXX([11], [noext], [mandatory], [nodefault])
dnl Check if -latomic is required for <std::atomic>
CHECK_ATOMIC

 # Initialize libtool.
LT_INIT([pic-only])

# Enable debug
AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug],
                    [use debug compiler flags and macros (default is no)])],
	[enable_debug=$enableval],
	[enable_debug=no])

if test "x$enable_debug" = xyes; then
	CXXFLAGS="$CXXFLAGS -g3 -O0"
fi

# Checks lockorder
AC_ARG_ENABLE([check-lockorder],
[AS_HELP_STRING([--enable-check-lockorder],
				[use checking lockorder macro (default is yes)])],
[enable_check_lockorder=$enableval],
[enable_check_lockorder=yes])

if test "x$enable_check_lockorder" = xyes; then
	CPPFLAGS="$CPPFLAGS -DCHECK_LOCKORDER"
fi

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.

AC_SUBST(BTCLITE_DAEMON_NAME)

AX_BOOST_BASE([1.47.0])
AX_BOOST_SYSTEM
AX_BOOST_THREAD
BOOST_LIBS="$BOOST_LDFLAGS $BOOST_SYSTEM_LIB $BOOST_THREAD_LIB"
AC_SUBST(BOOST_LIBS)

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
