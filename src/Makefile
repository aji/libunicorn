include ../extra.mk

LIB_MAJOR = 0
LIB_MINOR = 1

SHARED_LIB = libchat.so
STATIC_LIB =

INCLUDES = chat.h

SRCS = hook.c \
	isupport.c \
	log.c \
	message.c \
	mode.c \
	nick.c

include ../buildsys.mk

LIBS += ${libmowgli_2_LIBS}
CFLAGS += ${libmowgli_2_CFLAGS}
