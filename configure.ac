#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
define(_CLIENT_VERSION_MAJOR, 0)
define(_CLIENT_VERSION_MINOR, 1)
define(_CLIENT_VERSION_REVISION, 3)
AC_INIT([btcdemo], [_CLIENT_VERSION_MAJOR._CLIENT_VERSION_MINOR._CLIENT_VERSION_REVISION], [https://github.com/rongwl/btcdemo/issues])
AC_CONFIG_SRCDIR([src/validation.cpp])
AC_CONFIG_HEADERS([src/config/btcdemo-config.h])
AC_CONFIG_MACRO_DIR([m4])

BTCDEMO_DAEMON_NAME=bitdemod

AH_TOP([#ifndef BTCDEMO_CONFIG_H])
AH_TOP([#define BTCDEMO_CONFIG_H])
AH_BOTTOM([#endif])

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

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.

AC_SUBST(BTCDEMO_DAEMON_NAME)

AX_BOOST_BASE([1.47.0])
AX_BOOST_SYSTEM
AX_BOOST_THREAD
BOOST_LIBS="$BOOST_LDFLAGS $BOOST_SYSTEM_LIB $BOOST_THREAD_LIB"
AC_SUBST(BOOST_LIBS)

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
