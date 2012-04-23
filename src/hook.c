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

int irc_hook_table_destroy(irc_hook_table_t *table)
{
	mowgli_patricia_iteration_state_t state;
	irc_hook_t *hook;

	mowgli_patricia_foreach_start(table->hooks, &state);

	while ((hook = mowgli_patricia_foreach_cur(table->hooks, &state)) != NULL) {
		irc_hook_destroy_all(hook);
		mowgli_patricia_foreach_next(table->hooks, &state);
	}

	mowgli_free(table);

	return 0;
}


int irc_hook_add(irc_hook_table_t *table, char *hook, irc_hook_cb_t *cb, void *priv)
{
	irc_hook_t *newhook, *head;

	newhook = irc_hook_create(cb, priv);
	if (newhook == NULL)
		return -1;

	head = mowgli_patricia_delete(table->hooks, hook);

	newhook->next = head;

	if (!mowgli_patricia_add(table->hooks, hook, newhook))
		return -1;

	return 0;
}


int irc_hook_call(irc_hook_table_t *table, char *hook, int parc, char *parv[])
{
	irc_hook_t *curr;

	curr = mowgli_patricia_retrieve(table->hooks, hook);
	if (curr == NULL)
		return -1;

	while (curr) {
		if (curr->cb)
			curr->cb(parc, parv, curr->priv);

		curr = curr->next;
	}

	return 0;
}


int irc_hook_simple_dispatch(irc_hook_table_t *table, irc_message_t *msg)
{
	mowgli_node_t *n;
	int parc, i;
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

	i = irc_hook_call(table, msg->command, parc, parv);

	mowgli_free(parv);

	return i;
}
