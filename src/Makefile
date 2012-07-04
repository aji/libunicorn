include ../extra.mk

LIB_MAJOR = 0
LIB_MINOR = 4

SHARED_LIB = libunicorn.so
STATIC_LIB =

INCLUDES = unicorn.h

SRCS = hook.c \
	isupport.c \
	message.c \
	mode.c \
	casemap.c

include ../buildsys.mk

LIBS += ${libmowgli_2_LIBS}
CFLAGS += ${libmowgli_2_CFLAGS}
