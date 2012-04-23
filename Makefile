include extra.mk

SUBDIRS = src
DISTCLEAN = buildsys.mk extra.mk config.log config.status libunicorn.pc aclocal.m4 configure

include buildsys.mk
