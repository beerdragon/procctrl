#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([procctrl], [0.1.0])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([cunit], [CU_initialize_registry], [AC_SUBST([CUNIT_LDFLAGS], [-lcunit])])

# Checks for header files.
AC_CHECK_HEADER([CUnit/Basic.h], [AC_DEFINE([HAVE_CUNIT_H], 1, [Define to 1 for CUnit tests.])])
AC_CHECK_HEADERS([stdlib.h string.h sys/file.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([mkdir])
AC_CHECK_FUNCS([rmdir])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strtol])
AC_FUNC_FORK
AC_FUNC_MALLOC

AC_OUTPUT(Makefile src/Makefile man/Makefile)
