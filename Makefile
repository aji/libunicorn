include extra.mk

SUBDIRS = src
DISTCLEAN = buildsys.mk extra.mk config.log config.status libunicorn.pc aclocal.m4 configure config.guess config.sub install-sh

include buildsys.mk

PKGCONF = libunicorn.pc

# TODO: make this part of buildsys?
install-extra:
	i="${PKGCONF}"; \
	${INSTALL_STATUS}; \
	if ${MKDIR_P} ${DESTDIR}${libdir}/pkgconfig \
	&& ${INSTALL} -m 644 $$i ${DESTDIR}${libdir}/pkgconfig/$$i; then \
		${INSTALL_OK}; \
	else \
		${INSTALL_FAILED}; \
	fi

uninstall-extra:
	i="${PKGCONF}"; \
	if [ -f ${DESTDIR}${libdir}/pkgconfig/$$i ]; then \
		if rm -f ${DESTDIR}${libdir}/pkgconfig/$$i; then \
			${DELETE_OK}; \
		else \
			${DELETE_FAILED}; \
		fi \
	fi

