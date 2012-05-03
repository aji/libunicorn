include extra.mk

SUBDIRS = src
DISTCLEAN = buildsys.mk extra.mk config.log config.status libunicorn.pc aclocal.m4 configure config.guess config.sub install-sh

include buildsys.mk
