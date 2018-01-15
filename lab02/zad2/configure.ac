#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([libmatrixes], [0.0.2], [lewap@student.agh.edu.pl])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/matrixes.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
LT_INIT
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_MSG_CHECKING([whether to enable double])
AC_ARG_ENABLE(double,
	AC_HELP_STRING([--enable-double], [enable double support]),
	[                                                           
		if test "x$enable_double" = "xyes" ; then
			AC_DEFINE([DOUBLE], [], [Support for double])
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
		fi
	],
	AC_MSG_RESULT([no])
)

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
