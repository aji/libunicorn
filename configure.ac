dnl  To generate the configure script, call:
dnl    aclocal -I m4
dnl    autoconf

AC_INIT([libunicorn], [0.2-alpha1], [https://github.com/aji/libunicorn/issues])
AC_CONFIG_SRCDIR(src)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S

PKG_CHECK_MODULES([libmowgli_2], [libmowgli-2])

BUILDSYS_SHARED_LIB

BUILDSYS_INIT
BUILDSYS_TOUCH_DEPS

AC_CONFIG_FILES([buildsys.mk extra.mk libunicorn.pc])

AC_OUTPUT
