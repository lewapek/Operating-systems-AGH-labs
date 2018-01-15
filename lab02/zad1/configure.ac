#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([libmem_management], [0.0.1], [lewap@student.agh.edu.pl])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/mem_management.c])
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
AC_MSG_CHECKING([whether size of block is set])
AC_ARG_WITH(block-size,
	AS_HELP_STRING([--with-block-size], [Set block size in B (default is 1B)]),
	[
		AC_MSG_RESULT([yes])
		echo "block size set to $with_block_size B"
		AC_DEFINE_UNQUOTED([BLOCK_SIZE], [$with_block_size], [Block size in B])
	],
	[
		AC_MSG_RESULT([no])
		echo "default block size of 1kB will be used"
	]
)

AC_MSG_CHECKING([whether allocator strategy has been set])
AC_ARG_WITH(allocator,
	AS_HELP_STRING([--with-allocator], [Set to MIN_ALLOC_STRATEGY to allocate from the smallest node or MAX_ALLOC_STRATEGY from the largest node.]),
	[
		if test "$with_allocator" = "MAX_ALLOC_STRATEGY"; then
			AC_MSG_RESULT([yes])
			echo "strategy set to MAX_ALLOC_STRATEGY"
			AC_DEFINE([ALLOC_STRATEGY], [], [Allocation strategy is set])
			AC_DEFINE([MAX_ALLOC_STRATEGY], [], [Use MAX_ALLOC_STRATEGY strategy])
		else
			if test "$with_allocator" = "MIN_ALLOC_STRATEGY"; then
				AC_MSG_RESULT([yes])
				echo "strategy set to MIN_ALLOC_STRATEGY"
				AC_DEFINE([ALLOC_STRATEGY], [], [Allocation strategy is set])
				AC_DEFINE([MIN_ALLOC_STRATEGY], [], [Use MIN_ALLOC_STRATEGY strategy])
			else
				AC_MSG_RESULT([no])
				echo "invalid option"
			fi
		fi
	],
	[
		AC_MSG_RESULT([no])
		echo "Default strategy MIN_ALLOC_STRATEGY will be used"
	]
)

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
