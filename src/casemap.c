/*
 * libunicorn -- The magical IRC client library
 * Copyright (C) 2012 Alex Iadicicco
 *
 * This file is part of libunicorn and is protected under the terms
 * contained in the COPYING file in the project root.
 */

#include <ctype.h>
#include "unicorn.h"

static inline char map_null(char c)
{
	return c;
}

static inline char map_ascii(char c)
{
	return toupper(map_null(c));
}

static inline char map_strict_rfc1459(char c)
{
	c = map_ascii(c);
	switch (c) {
	case '{': return '[';
	case '}': return ']';
	case '|': return '\\';
	}
	return c;
}

static inline char map_rfc1459(char c)
{
	c = map_strict_rfc1459(c);
	if (c == '^')
		return '~';
	return c;
}

static inline int charcmp(char a, char b)
{
	return (int)a - (int)b;
}

int irc_casecmp(int casemapping, const char *a, const char *b)
{
	int c;

	/* TODO: macro-ify? */
	switch (casemapping) {
	case IRC_CASEMAPPING_ASCII:
		for (; ; a++, b++) {
			c = charcmp(map_ascii(*a), map_ascii(*b));
			if (c == 0 && *a)
				continue;
			return c;
		}
		break;
	case IRC_CASEMAPPING_STRICT_RFC1459:
		for (; ; a++, b++) {
			c = charcmp(map_strict_rfc1459(*a), map_strict_rfc1459(*b));
			if (c == 0 && *a)
				continue;
			return c;
		}
		break;
	case IRC_CASEMAPPING_RFC1459:
	default:
		for (; ; a++, b++) {
			c = charcmp(map_rfc1459(*a), map_rfc1459(*b));
			if (c == 0 && *a)
				continue;
			return c;
		}
		break;
	}
	return 0;
}

void irc_casemap_null(char *s)
{
}

void irc_casemap_ascii(char *s)
{
	do { *s = map_ascii(*s); } while (*++s);
}

void irc_casemap_rfc1459(char *s)
{
	do { *s = map_rfc1459(*s); } while (*++s);
}

void irc_casemap_strict_rfc1459(char *s)
{
	do { *s = map_strict_rfc1459(*s); } while (*++s);
}


void (*irc_casemap_fn(int casemapping))(char*)
{
	switch (casemapping) {
	case IRC_CASEMAPPING_ASCII:
		return irc_casemap_ascii;
	case IRC_CASEMAPPING_RFC1459:
		return irc_casemap_rfc1459;
	case IRC_CASEMAPPING_STRICT_RFC1459:
		return irc_casemap_strict_rfc1459;
	}
	return irc_casemap_rfc1459;
}
