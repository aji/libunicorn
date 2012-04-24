/*
 * libunicorn -- The magical IRC client library
 * Copyright (C) 2012 Alex Iadicicco
 *
 * This file is part of libunicorn and is protected under the terms
 * contained in the COPYING file in the project root.
 */

#include <stdio.h>
#include <stdarg.h>

#include "unicorn.h"


static const char *log_prefix[] =
	{ "DEBUG: ", "INFO: ", "NOTICE: ", "WARN: ", "ERROR: ", "FATAL: " };
static int log_level = IRC_LOG_DEBUG;


void irc_log_level(int level)
{
	log_level = level;
}

int irc_log(int level, char *fmt, ...)
{
	va_list va;
	int r;

	if (level < IRC_LOG_DEBUG)
		level = IRC_LOG_DEBUG;
	if (level > IRC_LOG_FATAL)
		level = IRC_LOG_FATAL;

	if (level < log_level)
		return 0;

	va_start(va, fmt);

	fputs(log_prefix[level], stderr);
	r = vfprintf(stderr, fmt, va);

	va_end(va);

	return r;
}

