/*
 * libunicorn -- The magical IRC client library
 * Copyright (C) 2012 Alex Iadicicco
 *
 * This file is part of libunicorn and is protected under the terms
 * contained in the COPYING file in the project root.
 */

#include <ctype.h>
#include "unicorn.h"

void irc_casemap_null(char *s)
{
}

void irc_casemap_ascii(char *s)
{
	while (*s) {
		*s = toupper(*s);
		s++;
	}
}

void irc_casemap_rfc1459(char *s)
{
	while (*s) {
		*s = toupper(*s);

		switch (*s) {
		case '{': *s = '['; break;
		case '}': *s = ']'; break;
		case '|': *s = '\\'; break;
		case '^': *s = '~'; break;
		}

		s++;
	}
}

void irc_casemap_strict_rfc1459(char *s)
{
	while (*s) {
		*s = toupper(*s);

		switch (*s) {
		case '{': *s = '['; break;
		case '}': *s = ']'; break;
		case '|': *s = '\\'; break;
		}

		s++;
	}
}
