/*
 * libunicorn -- The magical IRC client library
 * Copyright (C) 2012 Alex Iadicicco
 *
 * This file is part of libunicorn and is protected under the terms
 * contained in the COPYING file in the project root.
 */

#include <mowgli.h>
#include <ctype.h>
#include "unicorn.h"


irc_hook_t *irc_hook_create(irc_hook_cb_t *cb, void *priv)
{
	irc_hook_t *hook;

	hook = mowgli_alloc(sizeof(*hook));
	if (hook == NULL)
		return NULL;

	hook->cb = cb;
	hook->priv = priv;
	hook->next = NULL;

	return hook;
}

void irc_hook_destroy_all(irc_hook_t *hook)
{
	irc_hook_t *next;

	while (hook) {
		next = hook->next;
		mowgli_free(hook);
		hook = next;
	}
}


void irc_hook_table_canonize(char *str)
{
	while (*str) {
		*str = toupper(*str);
		str++;
	}
}

irc_hook_table_t *irc_hook_table_create()
{
	irc_hook_table_t *table;

	table = mowgli_alloc(sizeof(*table));
	if (table == NULL)
		return NULL;

	table->hooks = mowgli_patricia_create(irc_hook_table_canonize);
	if (table->hooks == NULL) {
		mowgli_free(table);
		return NULL;
	}

	return table;
}

void irc_hook_table_destroy_cb(const char *key, void *data, void *privdata)
{
	irc_hook_def_t *def = data;

	irc_hook_destroy_all(def->head);
	mowgli_free(def);
}
int irc_hook_table_destroy(irc_hook_table_t *table)
{
	mowgli_patricia_destroy(table->hooks, irc_hook_table_destroy_cb, NULL);
	mowgli_free(table);

	return 0;
}


int irc_hook_add(irc_hook_table_t *table, const char *hook, irc_hook_cb_t *cb, void *priv)
{
	irc_hook_t *newhook;
	irc_hook_def_t *def;

	newhook = irc_hook_create(cb, priv);
	if (newhook == NULL)
		return -1;

	def = mowgli_patricia_delete(table->hooks, hook);

	if (def == NULL) {
		def = mowgli_alloc(sizeof(*def));
		if (def == NULL)
			return -1;
		memset(def, 0, sizeof(*def));

		def->head = def->tail = newhook;
	} else {
		def->tail->next = newhook;
		def->tail = newhook;
	}

	if (!mowgli_patricia_add(table->hooks, hook, def))
		return -1;

	return 0;
}


int irc_hook_call(irc_hook_table_t *table, const char *hook, int parc, const char *parv[])
{
	irc_hook_t *curr;
	irc_hook_def_t *def;

	def = mowgli_patricia_retrieve(table->hooks, hook);
	if (def == NULL)
		return -1;

	curr = def->head;

	while (curr) {
		if (curr->cb) {
			if (curr->cb(parc, parv, curr->priv) != 0)
				break;
		}

		curr = curr->next;
	}

	return 0;
}


int irc_hook_simple_dispatch(irc_hook_table_t *table, irc_message_t *msg)
{
	return irc_hook_prefix_dispatch(table, msg, "");
}

int irc_hook_prefix_dispatch(irc_hook_table_t *table, irc_message_t *msg, const char *prefix)
{
	// TODO: deprecate this? prefixes can be accomplished with
	// multiple hook tables. The existence of this feature is
	// largely unjustified.
	mowgli_node_t *n;
	char *command;
	int parc, i;
	size_t len;
	char **parv;

	len = strlen(prefix) + strlen(msg->command) + 1;
	command = mowgli_alloc(len);
	memset(command, 0, len);
	strcpy(command, prefix);
	strcat(command, msg->command);

	parc = msg->args.count + 1;
	parv = mowgli_alloc_array(sizeof(char*), parc);

	if (msg->source.type == IRC_MESSAGE_SOURCE_NONE)
		parv[0] = "";
	else
		parv[0] = msg->source.user.nick;

	i = 1;
	MOWGLI_LIST_FOREACH(n, msg->args.head) {
		parv[i] = n->data;
		i++;
	}

	i = irc_hook_call(table, command, parc, (const char**)parv);

	mowgli_free(command);
	mowgli_free(parv);

	return i;
}

int irc_hook_ext_dispatch(irc_hook_table_t *table, irc_message_t *msg)
{
	// TODO: merge this with irc_hook_prefix_dispatch somehow?
	mowgli_node_t *n;
	int parc, i;
	size_t len;
	char **parv;

	parc = msg->args.count + 1;
	parv = mowgli_alloc_array(sizeof(char*), parc);

	parv[0] = parv[1] = parv[2] = NULL;
	if (msg->source.type == IRC_MESSAGE_SOURCE_SERVER) {
		parv[2] = msg->source.server.name;
	} else if (msg->source.type == IRC_MESSAGE_SOURCE_USER) {
		parv[0] = (msg->source.user.nick ? msg->source.user.nick : "");
		parv[1] = (msg->source.user.ident ? msg->source.user.ident : "");
		parv[2] = (msg->source.user.host ? msg->source.user.host : "");
	}

	i = 3;
	MOWGLI_LIST_FOREACH(n, msg->args.head) {
		parv[i] = n->data;
		i++;
	}

	i = irc_hook_call(table, msg->command, parc, (const char**)parv);

	mowgli_free(parv);

	return i;
}
