include configure.mk
include build.mk

SUBDIRS = src/

compile: ${SUBDIRS}
	@${SUBDIR_TARGET}
clean: ${SUBDIRS}
	@${SUBDIR_TARGET}
