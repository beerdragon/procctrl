#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([procctrl], [1.0.0])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADER(ctype.h)
AC_CHECK_HEADER(unistd.h)
AC_CHECK_HEADER(stdio.h)
AC_CHECK_HEADER(stdlib.h)
AC_CHECK_HEADER(string.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS(mkdir)
AC_CHECK_FUNCS(rmdir)
AC_CHECK_FUNCS(strdup)
AC_CHECK_FUNCS(strtol)
AC_FUNC_FORK
AC_FUNC_MALLOC

AC_OUTPUT(Makefile src/Makefile man/Makefile)
