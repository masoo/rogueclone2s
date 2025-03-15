#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([rogueclone2s],[7.0-preview1],[https://github.com/masoo/rogueclone2s])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([ncursesw], [initscr])
AC_CHECK_LIB([kernel32], [_sleep])

# Checks for required header files.
AC_CHECK_HEADERS([curses.h], [], [AC_MSG_ERROR([curses.h is required but not found.])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h sgtty.h stdlib.h stdint.h string.h strings.h sys/file.h sys/time.h termio.h unistd.h windows.h])
AC_CHECK_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM
AC_TYPE_SIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT32_T
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_STAT
AC_CHECK_FUNCS([assume_default_colors attr_get chdir getcwd getenv getlogin memset putenv setenv setlocale sleep strchr])

# Configuration options
AC_ARG_ENABLE([color],
              AS_HELP_STRING([--enable-color],
                             [include color (default yes)]), ,)

if test "$enable_color" != "no"; then
  AC_DEFINE([COLOR], , [Define if you have the 'COLOR'])
fi

# Define constants
AC_DEFINE([TOPSCO], , [Define if you have the 'TOPSCO'])
AC_DEFINE([THROW_TUNNEL], , [Define if you have the 'THROW_TUNNEL'])

# Output files
AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 man/Makefile
                 msg/Makefile
                 src/Makefile])

AC_OUTPUT
