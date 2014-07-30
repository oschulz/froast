# Initialization

AC_INIT([FRoASt], [0.3.0], [], [froast])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror])


# Libtool init and language selection

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext])
CXX_PROJECT_INIT


# Checks for dependencies

ROOTSYS_DEPS([], [-lTreePlayer])

CXX_PROJECT_DEPS([])


# Output

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
	Makefile
	froast.pc
	froast-config
	src/Makefile
	doc/Makefile doc/Doxyfile
])

AC_OUTPUT
