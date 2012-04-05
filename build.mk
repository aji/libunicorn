SUBDIR_ENTER = echo -e "\e[34;1mEntering directory $$i\e[0m"
SUBDIR_LEAVE = echo -e "\e[34;1mLeaving directory $$i\e[0m"

CLEAN_START = echo -e "\e[33;1mRemoving $$i...\e[0m"
CLEAN_FAIL = echo -e "\e[31;1mFailed to remove $$i\e[0m"
CLEAN_SUCCESS = true


COMPILE_START = echo -e "\e[32;1mCompiling $<...\e[0m"
COMPILE_FAIL = echo -e "\e[31;1mFailed to compile $<\e[0m"
COMPILE_SUCCESS = true

LINK_START = echo -e "\e[32;1mLinking $<...\e[0m"
LINK_FAIL = echo -e "\e[31;1mFailed to link $<\e[0m"
LINK_SUCCESS = true



.SUFFIXES: .c .o .clean

.c.o:
	@${COMPILE_START}
	@if ${CC} -c -o $@ $< ${CFLAGS}; then \
		${COMPILE_SUCCESS}; else \
		${COMPILE_FAIL}; fi

COMPILE_OBJECTS = \
	for i in ${OBJECTS}; do \
		make -q $$i || make --no-print-directory $$i; \
	done
CLEAN_OBJECTS = \
	for i in ${OBJECTS}; do \
		if [ -e "$$i" ]; then \
			${CLEAN_START}; \
			if ${RM} -f "$$i"; then \
				${CLEAN_SUCCESS}; else \
				${CLEAN_FAIL}; fi; \
		fi; \
	done
SUBDIR_TARGET = \
	for i in $^; do \
		${SUBDIR_ENTER}; \
		make --no-print-directory -C $$i $@; \
		${SUBDIR_LEAVE}; \
	done
