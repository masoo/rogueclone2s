#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(rogueclone2s, 1.0, http://homepage2.nifty.com/107steps/)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile
                 src/Makefile])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h sgtty.h sys/file.h sys/time.h termio.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_STRUCT_TM

# Checks for library functions.
AC_HEADER_STDC
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([getcwd memset putenv setenv strchr])
AC_OUTPUT
