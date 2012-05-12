include ../extra.mk

LIB_MAJOR = 0
LIB_MINOR = 2

SHARED_LIB = libunicorn.so
STATIC_LIB =

INCLUDES = unicorn.h

SRCS = hook.c \
	isupport.c \
	log.c \
	message.c \
	mode.c \
	nick.c

include ../buildsys.mk

LIBS += ${libmowgli_2_LIBS}
CFLAGS += ${libmowgli_2_CFLAGS}
