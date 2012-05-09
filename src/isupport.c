/*
 * libunicorn -- The magical IRC client library
 * Copyright (C) 2012 Alex Iadicicco
 *
 * This file is part of libunicorn and is protected under the terms
 * contained in the COPYING file in the project root.
 */

#include <string.h>
#include <mowgli.h>
#include "unicorn.h"

// function naming issue? ... maybe
void mowgli_string_copy(mowgli_string_t *self, const char *src, size_t n)
{
	self->reset(self);
	self->append(self, src, n);
}


void irc_isupport_values_canonize(char *key)
{
	while (*key) {
		*key = toupper(*key);
		key++;
	}
}
irc_isupport_t *irc_isupport_create(void)
{
	irc_isupport_t *isupport;

	isupport = mowgli_alloc(sizeof(*isupport));

	isupport->chanmodes.list = mowgli_string_create();
	isupport->chanmodes.arg_always = mowgli_string_create();
	isupport->chanmodes.arg_onset = mowgli_string_create();
	isupport->chanmodes.noarg = mowgli_string_create();

	isupport->chantypes = mowgli_string_create();

	isupport->prefix.mode = mowgli_string_create();
	isupport->prefix.prefix = mowgli_string_create();

	isupport->values = mowgli_patricia_create(irc_isupport_values_canonize);

	irc_isupport_reset(isupport);

	return isupport;
}

void irc_isupport_destroy_cb(const char *key, void *data, void *privdata)
{
	mowgli_free(data);
}
int irc_isupport_destroy(irc_isupport_t *isupport)
{
	mowgli_string_destroy(isupport->chanmodes.list);
	mowgli_string_destroy(isupport->chanmodes.arg_always);
	mowgli_string_destroy(isupport->chanmodes.arg_onset);
	mowgli_string_destroy(isupport->chanmodes.noarg);

	mowgli_string_destroy(isupport->chantypes);

	mowgli_string_destroy(isupport->prefix.mode);
	mowgli_string_destroy(isupport->prefix.prefix);

	mowgli_patricia_destroy(isupport->values, irc_isupport_destroy_cb, NULL);

	return 0;
}

int irc_isupport_reset(irc_isupport_t *isupport)
{
	isupport->casemapping = IRC_ISUPPORT_CASEMAPPING_RFC1459;

	// TODO: chanmodes

	mowgli_string_copy(isupport->chantypes, "#", 1);

	isupport->modes = 1;
	isupport->nicklen = 8;

	mowgli_string_copy(isupport->prefix.mode, "ov", 2);
	mowgli_string_copy(isupport->prefix.prefix, "@+", 2);

	return 0;
}


char *irc_isupport_strtok(char **buf, int chr)
{
	char *at;
	char *ret = *buf;

	at = strchr(*buf, chr);

	if (at == NULL) {
		*buf = strchr(*buf, '\0');
		return NULL;
	}

	*at = '\0';
	*buf = at + 1;

	return ret;
}

int irc_isupport_casemapping(irc_isupport_t *isupport, char *value)
{
	// default to RFC 1459 casemapping
	isupport->casemapping = IRC_ISUPPORT_CASEMAPPING_RFC1459;

	if (value == NULL)
		return -1;

	if (!strcmp(value, "ascii"))
		isupport->casemapping = IRC_ISUPPORT_CASEMAPPING_ASCII;
	else if (!strcmp(value, "rfc1459"))
		isupport->casemapping = IRC_ISUPPORT_CASEMAPPING_RFC1459;
	else if (!strcmp(value, "strict-rfc1459"))
		isupport->casemapping = IRC_ISUPPORT_CASEMAPPING_STRICT_RFC1459;

	return 0;
}

int irc_isupport_chanmodes(irc_isupport_t *isupport, char *value)
{
	char *a, *b, *c, *d;
	size_t n;

	if (value == NULL)
		return -1;

	d = value;
	a = irc_isupport_strtok(&d, ',');
	b = irc_isupport_strtok(&d, ',');
	c = irc_isupport_strtok(&d, ',');

	if (a == NULL || b == NULL || c == NULL || d[0] == '\0')
		return -1;

	if (isupport->chanmodes.list)
		mowgli_string_copy(isupport->chanmodes.list, a, strlen(a));
	if (isupport->chanmodes.arg_always)
		mowgli_string_copy(isupport->chanmodes.arg_always, b, strlen(b));
	if (isupport->chanmodes.arg_onset)
		mowgli_string_copy(isupport->chanmodes.arg_onset, c, strlen(c));
	if (isupport->chanmodes.noarg)
		mowgli_string_copy(isupport->chanmodes.noarg, d, strlen(d));

	return 0;
}

int irc_isupport_chantypes(irc_isupport_t *isupport, char *value)
{
	if (value == NULL)
		return -1;

	if (isupport->chantypes)
		mowgli_string_copy(isupport->chantypes, value, strlen(value));

	return 0;
}

int irc_isupport_modes(irc_isupport_t *isupport, char *value)
{
	if (value == NULL)
		return -1;

	isupport->modes = atoi(value);
	return 0;
}

int irc_isupport_nicklen(irc_isupport_t *isupport, char *value)
{
	if (value == NULL)
		return -1;

	isupport->nicklen = atoi(value);
	return 0;
}

int irc_isupport_prefix(irc_isupport_t *isupport, char *value)
{
	char *a, *b;
	size_t n;

	if (value == NULL)
		return -1;

	value++;
	a = strtok_r(value, ")", &b);

	if (a == NULL)
		return -1;

	if (isupport->prefix.mode)
		mowgli_string_copy(isupport->prefix.mode, a, strlen(a));
	if (isupport->prefix.prefix)
		mowgli_string_copy(isupport->prefix.prefix, b, strlen(b));

	return 0;
}

int irc_isupport_parse(irc_isupport_t *isupport, irc_message_t *msg)
{
	char *param, *value;
	int error = 0;
	mowgli_node_t *curr;

	int (*cb)(irc_isupport_t*, char*);

	// first a simple sanity check
	if (strcmp(msg->command, "005"))
		return -1;

	// then iterate over each arg
	MOWGLI_LIST_FOREACH(curr, msg->args.head) {
		// Nickname is always first arg, and "are supported by
		// this server" is always last. Ignore both
		if (curr->prev == NULL || curr->next == NULL)
			continue;

		param = curr->data;
		value = strchr(param, '=');
		if (value)
			*value++ = '\0';

		if (mowgli_patricia_retrieve(isupport->values, param) != NULL)
			mowgli_free(mowgli_patricia_delete(isupport->values, param));
		mowgli_patricia_add(isupport->values, param, mowgli_strdup(value ? value : ""));

		cb = NULL;

		if (!strcmp(param, "CASEMAPPING"))
			cb = irc_isupport_casemapping;
		else if (!strcmp(param, "CHANMODES"))
			cb = irc_isupport_chanmodes;
		else if (!strcmp(param, "CHANTYPES"))
			cb = irc_isupport_chantypes;
		else if (!strcmp(param, "MODES"))
			cb = irc_isupport_modes;
		else if (!strcmp(param, "NICKLEN"))
			cb = irc_isupport_nicklen;
		else if (!strcmp(param, "PREFIX"))
			cb = irc_isupport_prefix;

		if (cb)
			error = cb(isupport, value) < 0 ? -1 : error;
	}

	return 0;
}


char *irc_isupport_get_prefix_mode(irc_isupport_t *isupport)
{
	if (isupport == NULL || isupport->prefix.mode == NULL
			|| isupport->prefix.mode->str == NULL)
		return "ov";

	return isupport->prefix.mode->str;
}

char *irc_isupport_get_prefix_char(irc_isupport_t *isupport)
{
	if (isupport == NULL || isupport->prefix.prefix == NULL
			|| isupport->prefix.prefix->str == NULL)
		return "@+";

	return isupport->prefix.prefix->str;
}

