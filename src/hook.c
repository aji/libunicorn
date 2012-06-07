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


irc_hook_t *irc_hook_create(irc_hook_cb_t *cb)
{
	irc_hook_t *hook;

	hook = mowgli_alloc(sizeof(*hook));
	if (hook == NULL)
		return NULL;

	// LOL THIS IS RETARDED
	hook->cb = cb;

	return hook;
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
	mowgli_list_t *list = data;
	mowgli_node_t *n, *tn;

	MOWGLI_LIST_FOREACH_SAFE(n, tn, list->head) {
		mowgli_free(n->data);
		mowgli_node_delete(n, list);
		mowgli_node_free(n);
	}

	mowgli_list_free(list);
}
int irc_hook_table_destroy(irc_hook_table_t *table)
{
	mowgli_patricia_destroy(table->hooks, irc_hook_table_destroy_cb, NULL);
	mowgli_free(table);

	return 0;
}


int irc_hook_add(irc_hook_table_t *table, const char *hookname, irc_hook_cb_t *cb)
{
	irc_hook_t *newhook;
	mowgli_list_t *list;

	newhook = irc_hook_create(cb);
	if (newhook == NULL)
		return -1;

	list = mowgli_patricia_delete(table->hooks, hookname);

	if (list == NULL) {
		list = mowgli_list_create();
		if (list == NULL)
			return -1;
	}

	mowgli_node_add(newhook, mowgli_node_create(), list);

	if (!mowgli_patricia_add(table->hooks, hookname, list))
		return -1;

	return 0;
}

int irc_hook_del(irc_hook_table_t *table, const char *hookname, irc_hook_cb_t *cb)
{
	mowgli_list_t *list;
	mowgli_node_t *n;
	irc_hook_t *hook;

	list = mowgli_patricia_delete(table->hooks, hookname);

	if (list == NULL)
		return 0;

	MOWGLI_LIST_FOREACH(n, list->head) {
		hook = n->data;

		if (hook->cb == cb) {
			mowgli_free(hook);
			mowgli_node_delete(n, list);
			mowgli_node_free(n);
			break;
		}
	}

	if (list->count == 0) {
		mowgli_list_free(list);
	} else {
		if (!mowgli_patricia_add(table->hooks, hookname, list))
			return -1;
	}

	return 0;
}


int irc_hook_call(irc_hook_table_t *table, const char *hookname, int parc, const char *parv[], void *ctx)
{
	mowgli_list_t *list;
	mowgli_node_t *n;
	irc_hook_t *hook;

	list = mowgli_patricia_retrieve(table->hooks, hookname);
	if (list == NULL)
		return -1;

	MOWGLI_LIST_FOREACH(n, list->head) {
		hook = n->data;

		if (hook->cb)
			if (hook->cb(parc, parv, ctx) != 0)
				break;
	}

	return 0;
}


int irc_hook_simple_dispatch(irc_hook_table_t *table, irc_message_t *msg, void *ctx)
{
	mowgli_node_t *n;
	int parc, i;
	size_t len;
	char **parv;

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

	i = irc_hook_call(table, msg->command, parc, (const char**)parv, ctx);

	mowgli_free(parv);

	return i;
}

int irc_hook_ext_dispatch(irc_hook_table_t *table, irc_message_t *msg, void *ctx)
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

	i = irc_hook_call(table, msg->command, parc, (const char**)parv, ctx);

	mowgli_free(parv);

	return i;
}
