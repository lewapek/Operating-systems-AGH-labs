#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(main, 1.0, lewap@student.agh.edu.pl)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strstr])

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

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
