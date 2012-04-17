#include <string.h>
#include <mowgli.h>

#include "unicorn.h"


// Peer

irc_client_peer_t *irc_client_peer_create(char *nick)
{
	irc_client_peer_t *peer;

	peer = mowgli_alloc(sizeof(*peer));
	if (peer == NULL)
		return NULL;

	peer->nick = mowgli_string_create();
	if (peer->nick == NULL) {
		mowgli_free(peer);
		return NULL;
	}

	peer->nick->reset(peer->nick);
	peer->nick->append(peer->nick, nick, strlen(nick));

	peer->ref = 0;

	return peer;
}

int irc_client_peer_destroy(irc_client_peer_t *peer)
{
	if (peer == NULL)
		return -1;

	mowgli_string_destroy(peer->nick);
	mowgli_free(peer);

	return 0;
}

void irc_client_peer_ref(irc_client_peer_t *peer)
{
	if (peer != NULL)
		peer->ref += 1;
}

void irc_client_peer_unref(irc_client_peer_t *peer)
{
	if (peer == NULL)
		return;

	peer->ref -= 1;

	if (peer->ref <= 0)
		irc_client_peer_destroy(peer);
}


// Channel users

irc_client_channel_user_t *irc_client_channel_user_create(irc_client_peer_t *peer, char prefix)
{
	irc_client_channel_user_t *user;

	user = mowgli_alloc(sizeof(*user));
	if (user == NULL)
		return NULL;

	user->peer = peer;
	user->prefix = prefix;

	irc_client_peer_ref(user->peer);

	return user;
}

int irc_client_channel_user_destroy(irc_client_channel_user_t *user)
{
	if (user == NULL)
		return -1;

	irc_client_peer_unref(user->peer);

	mowgli_free(user);

	return 0;
}


// Channels

irc_client_channel_t *irc_client_channel_create(char *name)
{
	irc_client_channel_t *channel;

	channel = mowgli_alloc(sizeof(*channel));
	if (channel == NULL)
		goto fail_alloc;

	channel->name = mowgli_string_create();
	if (channel->name == NULL)
		goto fail_name;

	channel->topic = mowgli_string_create();
	if (channel->topic == NULL)
		goto fail_topic;

	channel->users = mowgli_list_create();
	if (channel->users == NULL)
		goto fail_users;

	return channel;

fail_users:
	mowgli_string_destroy(channel->topic);
fail_topic:
	mowgli_string_destroy(channel->name);
fail_name:
	mowgli_free(channel);
fail_alloc:
	return NULL;
}

int irc_client_channel_destroy(irc_client_channel_t *channel)
{
	mowgli_node_t *n, *tn;

	if (channel == NULL)
		return -1;

	if (channel->name != NULL)
		mowgli_string_destroy(channel->name);
	if (channel->topic != NULL)
		mowgli_string_destroy(channel->topic);
	if (channel->users != NULL) {
		MOWGLI_LIST_FOREACH_SAFE(n, tn, channel->users->head) {
			irc_client_channel_user_destroy(n->data);
			mowgli_node_free(n);
		}

		mowgli_list_free(channel->users);
	}

	mowgli_free(channel);

	return 0;
}

int irc_client_channel_join(irc_client_channel_t *channel, irc_client_peer_t *peer, char prefix)
{
	irc_client_channel_user_t *user;

	if (channel == NULL || peer == NULL)
		return -1;

	user = irc_client_channel_user_create(peer, prefix);
	if (user == NULL)
		return -1;

	mowgli_node_add(user, mowgli_node_create(), channel->users);

	return 0;
}

int irc_client_channel_part(irc_client_channel_t *channel, irc_client_peer_t *peer)
{
	mowgli_node_t *n, *tn;

	if (channel == NULL || peer == NULL)
		return -1;

	MOWGLI_LIST_FOREACH_SAFE(n, tn, channel->users->head) {
		if (n->data == peer) {
			irc_client_channel_user_destroy(n->data);
			mowgli_node_delete(n, channel->users);
			mowgli_node_free(n);

			return 0;
		}
	}

	return -1;
}


// Client

irc_client_t *irc_client_create(void)
{
	irc_client_t *client;

	client = mowgli_alloc(sizeof(*client));
	if (client == NULL)
		goto fail_alloc;

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

	return client;


fail_channels:
	mowgli_patricia_destroy(client->peers, NULL, NULL);
fail_peers:
	mowgli_string_destroy(client->nick);
fail_nick:
	mowgli_free(client);
fail_alloc:
	return NULL;
}

void irc_client_deinit_peers_cb(const char *key, void *data, void *unused)
{
	irc_client_peer_unref(data);
}
void irc_client_deinit_channels_cb(const char *key, void *data, void *unused)
{
	irc_client_channel_destroy(data);
}

int irc_client_destroy(irc_client_t *client)
{
	if (client == NULL)
		return -1;

	if (client->nick != NULL)
		mowgli_string_destroy(client->nick);
	if (client->peers != NULL)
		mowgli_patricia_destroy(client->peers, irc_client_deinit_peers_cb, NULL);
	if (client->channels != NULL)
		mowgli_patricia_destroy(client->channels, irc_client_deinit_channels_cb, NULL);

	mowgli_free(client);

	return 0;
}


// Client Actions

int irc_client_do_join(irc_client_t *client, char *chan)
{
	irc_client_channel_t *channel;

	channel = mowgli_patricia_retrieve(client->channels, chan);
	if (channel != NULL)
		return -1;

	channel = irc_client_channel_create(chan);
	if (channel == NULL)
		return -1;

	mowgli_patricia_add(client->channels, chan, channel);

	return 0;
}

int irc_client_do_part(irc_client_t *client, char *chan)
{
	irc_client_channel_t *channel;

	channel = mowgli_patricia_retrieve(client->channels, chan);
	if (channel == NULL)
		return -1;

	mowgli_patricia_delete(client->channels, chan);

	return irc_client_channel_destroy(channel);
}

int irc_client_do_nick(irc_client_t *client, char *nick)
{
	client->nick->reset(client->nick);
	client->nick->append(client->nick, nick, strlen(nick));

	return 0;
}


int irc_client_peer_join(irc_client_t *client, char *nick, char *chan)
{
	irc_client_channel_t *channel;
	irc_client_peer_t *peer;

	channel = mowgli_patricia_retrieve(client->channels, chan);
	if (channel == NULL)
		return -1;

	peer = mowgli_patricia_retrieve(client->peers, nick);
	if (peer == NULL) {
		peer = irc_client_peer_create(nick);
		mowgli_patricia_add(client->peers, nick, peer);
	}

	return irc_client_channel_join(channel, peer, ' ');
}

int irc_client_peer_part(irc_client_t *client, char *nick, char *chan)
{
	irc_client_channel_t *channel;
	irc_client_peer_t *peer;

	channel = mowgli_patricia_retrieve(client->channels, chan);
	peer = mowgli_patricia_retrieve(client->peers, nick);

	// irc_client_channel_part will perform this check for us.
	// yeah, it's a rather pointless optimization...
	//if (channel == NULL || peer == NULL)
	//	return -1;

	return irc_client_channel_part(channel, peer);
}

int irc_client_peer_nick(irc_client_t *client, char *oldnick, char *newnick)
{
	irc_client_peer_t *peer;

	peer = mowgli_patricia_retrieve(client->peers, oldnick);
	if (peer == NULL || peer->nick == NULL)
		return -1;

	peer->nick->reset(peer->nick);
	peer->nick->append(peer->nick, newnick, strlen(newnick));

	return 0;
}


// Client message processing

int irc_client_process_message_server(irc_client_t *client, irc_message_t *msg)
{
	char *my_nick = NULL;

	if (strlen(msg->command) == 3 && isdigit(msg->command[0]) &&
				isdigit(msg->command[1]) && isdigit(msg->command[2])) {
		// snag the nickname from numerics
		my_nick = (char*)msg->args.head->data;
	}

	if (!strcmp(msg->command, "005")) {
		irc_isupport_parse(client->isupport, msg);
	}

	// TODO: process channel information (names 353 and topic 332/331)

	if (my_nick != NULL && strcmp(my_nick, client->nick->str) != 0)
		return irc_client_do_nick(client, my_nick);

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
	char *peer, *first_arg;

	peer = msg->source.user.nick;
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

