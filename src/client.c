#include <string.h>
#include <mowgli.h>

#include "unicorn.h"


int irc_client_init(irc_client_t *client)
{
	memset(client, 0, sizeof(*client));


	client->nick = mowgli_string_create();
	if (client->nick == NULL)
		goto fail_nick;


	client->peers = mowgli_patricia_create(NULL);
	if (client->peers == NULL)
		goto fail_peers;

	client->channels = mowgli_patricia_create(NULL);
	if (client->channels == NULL)
		goto fail_channels;

	return 0;


fail_channels:
	mowgli_patricia_destroy(client->peers);
fail_peers:
	mowgli_string_destroy(client->nick);
fail_nick:
	memset(client, 0, sizeof(*client));
	return -1;
}

int irc_client_deinit(irc_client_t *client)
{
	if (client->nick != NULL)
		mowgli_string_destroy(client->nick);
	if (client->peers != NULL)
		mowgli_patricia_destroy(client->peers);
	if (client->channels != NULL)
		mowgli_patricia_destroy(client->peers);

	client->nick = client->peers = client->channels = NULL;

	return 0;
}

irc_client_t *irc_client_create(void)
{
	irc_client_t *client;

	client = mowgli_alloc(sizeof(*client));
	if (client == NULL)
		return NULL;

	if (irc_client_init(client) < 0) {
		mowgli_free(client);
		return NULL;
	}

	return client;
}

int irc_client_destroy(irc_client_t *client)
{
	if (irc_client_deinit(client) < 0)
		return -1;

	mowgli_free(client);

	return 0;
}


int irc_client_do_join(irc_client_t *client, char *nick)
{
	// fetch channel, bail if exists
	// create empty channel
}

int irc_client_do_part(irc_client_t *client, char *nick)
{
	// fetch channel, bail if doesn't exist
	// free peers for which this is the last channel
	// free channel data
}

int irc_client_do_nick(irc_client_t *client, char *nick)
{
	// change nick
}


int irc_client_peer_join(irc_client_t *client, char *peer, char *chan)
{
	// fetch channel, bail if doesn't exist
	// fetch peer object, create if doesn't exist
	// add peer object to channel
}

int irc_client_peer_part(irc_client_t *client, char *peer, char *chan)
{
	// fetch channel, bail if doesn't exist
	// fetch peer object, bail if doesn't exist
	// remove peer from channel	
	// free peer if this is their last channel
}

int irc_client_peer_nick(irc_client_t *client, char *peer, char *nick)
{
	// fetch peer object, bail if doesn't exist
	// change peer nick
}


int irc_client_process_message_server(irc_client_t *client, irc_message_t *msg)
{
	if (strlen(msg->command) == 3 && isdigit(msg->command[0]) &&
				isdigit(msg->command[1]) && isdigit(msg->command[2])) {
		// snag the nickname from numerics
		return irc_client_do_nick(client, (char *)msg->args.head->data;
	}

	// TODO: process channel information (names 353 and topic 332/331)

	return 0;
}

int irc_client_process_message_self(irc_client_t *client, irc_message_t *msg)
{
	char *first_arg;

	first_arg = (char*)msg->args.head->data;

	if (!strcmp(msg->command, "JOIN")) {
		return irc_client_do_join(client, first_arg);
	} else if (!strcmp(msg->command, "PART")) {
		return irc_client_do_part(client, first_arg);
	}

	else if (!strcmp(msg->command, "NICK")) {
		return irc_client_do_nick(client, first_arg);
	}

	return 0;
}

int irc_client_process_message_peer(irc_client_t *client, irc_message_t *msg)
{
	irc_client_peer_t *peer;
	char *first_arg;

	peer = irc_client_find_peer(client, msg->source.user.nick);
	first_arg = (char*)msg->args.head->data;

	if (!strcmp(msg->command, "JOIN")) {
		return irc_client_peer_join(client, peer, first_arg);
	} else if (!strcmp(msg->command, "PART")) {
		return irc_client_peer_part(client, peer, first_arg);
	}

	else if (!strcmp(msg->command, "NICK")) {
		return irc_client_peer_nick(client, peer, first_arg);
	}

	return 0;
}


int irc_client_process_message(irc_client_t *client, irc_message_t *msg)
{
	switch (msg->source.type) {
	case IRC_MESSAGE_SOURCE_NONE:
		// We currently do not do anything with outgoing messages.
		break;

	case IRC_MESSAGE_SOURCE_SERVER:
		return irc_client_process_message_server(client, msg);
		break;

	case IRC_MESSAGE_SOURCE_USER:
		if (client->nick_cmp(client->nick->str, msg->source.user.nick) == 0) {
			return irc_client_process_message_self(client, msg);
		} else {
			return irc_client_process_message_peer(client, msg);
		}
		break;
	}

	return 0;
}

