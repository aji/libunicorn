/*
 * libunicorn -- The magical IRC client library
 * Copyright (C) 2012 Alex Iadicicco
 *
 * This file is part of libunicorn and is protected under the terms
 * contained in the COPYING file in the project root.
 */

#include <ctype.h>
#include "unicorn.h"

void irc_nick_canonize_null(char *nick)
{
}

void irc_nick_canonize_ascii(char *nick)
{
	while (*nick) {
		*nick = toupper(*nick);
		nick++;
	}
}

void irc_nick_canonize_rfc1459(char *nick)
{
	while (*nick) {
		*nick = toupper(*nick);

		switch (*nick) {
		case '{': *nick = '['; break;
		case '}': *nick = ']'; break;
		case '|': *nick = '\\'; break;
		case '^': *nick = '~'; break;
		}

		nick++;
	}
}

void irc_nick_canonize_strict_rfc1459(char *nick)
{
	while (*nick) {
		*nick = toupper(*nick);

		switch (*nick) {
		case '{': *nick = '['; break;
		case '}': *nick = ']'; break;
		case '|': *nick = '\\'; break;
		}

		nick++;
	}
}
