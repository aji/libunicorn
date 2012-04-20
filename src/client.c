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

int irc_client_peer_refcnt(irc_client_peer_t *peer)
{
	return peer->ref;
}


// Channel users

irc_client_channel_user_t *irc_client_channel_user_create(irc_client_peer_t *peer)
{
	irc_client_channel_user_t *user;

	user = mowgli_alloc(sizeof(*user));
	if (user == NULL)
		return NULL;

	// TODO: snag this from client->isupport
	user->prefix = irc_prefix_create(NULL);

	if (user->prefix == NULL) {
		mowgli_free(user);
		return NULL;
	}

	user->peer = peer;
	irc_client_peer_ref(user->peer);

	return user;
}

int irc_client_channel_user_destroy(irc_client_channel_user_t *user)
{
	if (user == NULL)
		return -1;

	if (user->prefix != NULL)
		irc_prefix_destroy(user->prefix);

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

int irc_client_channel_join(irc_client_channel_t *channel, irc_client_peer_t *peer, char mode)
{
	irc_client_channel_user_t *user;

	if (channel == NULL || peer == NULL)
		return -1;

	user = irc_client_channel_user_create(peer);
	if (user == NULL)
		return -1;

	if (mode != ' ')
		irc_prefix_set(user->prefix, mode);

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

int irc_client_init(irc_client_t *client)
{
	if (client == NULL)
		return -1;

	memset(client, 0, sizeof(*client));

	client->nick = mowgli_string_create();
	if (client->nick == NULL)
		goto fail_nick;

	client->peers = mowgli_patricia_create(&irc_nick_canonize_rfc1459);
	if (client->peers == NULL)
		goto fail_peers;

	client->channels = mowgli_patricia_create(NULL);
	if (client->channels == NULL)
		goto fail_channels;

	client->casemap = &irc_nick_canonize_rfc1459;

	return 0;


fail_channels:
	mowgli_patricia_destroy(client->peers, NULL, NULL);
fail_peers:
	mowgli_string_destroy(client->nick);
fail_nick:
	return -1;
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

void irc_client_deinit_peers_cb(const char *key, void *data, void *unused)
{
	irc_client_peer_unref(data);
}
void irc_client_deinit_channels_cb(const char *key, void *data, void *unused)
{
	irc_client_channel_destroy(data);
}

int irc_client_deinit(irc_client_t *client)
{
	if (client == NULL)
		return -1;

	if (client->nick != NULL)
		mowgli_string_destroy(client->nick);
	if (client->peers != NULL)
		mowgli_patricia_destroy(client->peers, irc_client_deinit_peers_cb, NULL);
	if (client->channels != NULL)
		mowgli_patricia_destroy(client->channels, irc_client_deinit_channels_cb, NULL);

	return 0;
}

int irc_client_destroy(irc_client_t *client)
{
	if (irc_client_deinit(client) < 0)
		return -1;

	mowgli_free(client);

	return 0;
}

int irc_client_set_casemapping(irc_client_t *client, int casemapping)
{
	void (*canonize_cb)(char *key);

	if (client == NULL)
		return -1;

	// We will not destroy a non-empty peer list. Sorry :(
	if (client->peers != NULL && mowgli_patricia_size(client->peers) != 0)
		return -1;

	canonize_cb = &irc_nick_canonize_rfc1459;

	switch (casemapping) {
	case IRC_ISUPPORT_CASEMAPPING_ASCII:
		irc_log_debug("client: using 'ascii' casemapping\n");
		canonize_cb = &irc_nick_canonize_ascii;
		break;

	case IRC_ISUPPORT_CASEMAPPING_RFC1459:
		irc_log_debug("client: using 'rfc1459' casemapping\n");
		canonize_cb = &irc_nick_canonize_rfc1459;
		break;

	case IRC_ISUPPORT_CASEMAPPING_STRICT_RFC1459:
		irc_log_debug("client: using 'strict-rfc1459' casemapping\n");
		canonize_cb = &irc_nick_canonize_strict_rfc1459;
		break;

	default:
		irc_log_debug("client: unknown casemapping, using 'rfc1459'\n");
	}

	if (client->peers != NULL)
		mowgli_patricia_destroy(client->peers, NULL, NULL);

	client->peers = mowgli_patricia_create(canonize_cb);
	client->casemap = canonize_cb;

	return (client->peers == NULL) ? -1 : 0;
}


// Client Actions

int irc_client_do_join(irc_client_t *client, char *chan)
{
	irc_client_channel_t *channel;

	irc_log_info("client: join channel %s\n", chan);

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

	irc_log_info("client: leaving channel %s\n", chan);

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

	irc_log_info("client: changed nickname to %s\n", nick);

	return 0;
}

int irc_client_do_quit(irc_client_t *client)
{
	irc_log_info("client: quitting\n");

	if (irc_client_deinit(client) < 0)
		return -1;

	return irc_client_init(client);
}

int irc_client_set_topic(irc_client_t *client, char *chan, char *topic)
{
	irc_client_channel_t *channel;

	irc_log_info("client: setting %s topic to '%s'\n", chan, topic);

	channel = mowgli_patricia_retrieve(client->channels, chan);
	if (channel == NULL)
		return -1;

	channel->topic->reset(channel->topic);
	channel->topic->append(channel->topic, topic, strlen(topic));

	return 0;
}


// if nick is prefixed by a single prefix char then it is used as the initial prefix
int irc_client_peer_join(irc_client_t *client, char *nick, char *chan)
{
	char *p, *q;
	irc_client_channel_t *channel;
	irc_client_peer_t *peer;

	irc_log_info("client: add peer %s to %s\n", nick, chan);

	channel = mowgli_patricia_retrieve(client->channels, chan);
	if (channel == NULL)
		return -1;

	p = irc_isupport_get_prefix_char(client->isupport);
	if ((q = strchr(p, *nick)) != NULL) {
		p = irc_isupport_get_prefix_mode(client->isupport) + (q - p);
		nick++;
	} else {
		p = " ";
	}

	peer = mowgli_patricia_retrieve(client->peers, nick);
	if (peer == NULL) {
		peer = irc_client_peer_create(nick);
		irc_client_peer_ref(peer);
		mowgli_patricia_add(client->peers, nick, peer);
	}

	return irc_client_channel_join(channel, peer, *p);
}

int irc_client_peer_part(irc_client_t *client, char *nick, char *chan)
{
	irc_client_channel_t *channel;
	irc_client_peer_t *peer;

	irc_log_info("client: remove peer %s from %s\n", nick, chan);

	channel = mowgli_patricia_retrieve(client->channels, chan);
	peer = mowgli_patricia_retrieve(client->peers, nick);

	if (channel == NULL || peer == NULL || irc_client_channel_part(channel, peer) < 0)
		return -1;

	if (irc_client_peer_refcnt(peer) == 1) {
		irc_client_peer_unref(peer);
		mowgli_patricia_delete(client->peers, nick);
	}

	return 0;
}

int irc_client_peer_quit_foreach(const char *key, void *data, void *privdata)
{
	irc_client_channel_part(data, privdata);
	return 0;
}
int irc_client_peer_quit(irc_client_t *client, char *nick)
{
	irc_client_peer_t *peer;

	peer = mowgli_patricia_retrieve(client->peers, nick);

	if (peer == NULL)
		return -1;

	mowgli_patricia_foreach(client->channels, &irc_client_peer_quit_foreach, peer);

	irc_client_peer_unref(peer);
	mowgli_patricia_delete(client->peers, nick);

	return 0;
}

int irc_client_peer_nick(irc_client_t *client, char *oldnick, char *newnick)
{
	irc_client_peer_t *peer;

	irc_log_info("client: peer %s is now known as %s\n", oldnick, newnick);

	peer = mowgli_patricia_retrieve(client->peers, oldnick);
	if (peer == NULL || peer->nick == NULL)
		return -1;

	peer->nick->reset(peer->nick);
	peer->nick->append(peer->nick, newnick, strlen(newnick));

	mowgli_patricia_delete(client->peers, oldnick);
	mowgli_patricia_add(client->peers, newnick, peer);

	return 0;
}


// Client message processing

int irc_client_message_is_me(irc_client_t *client, irc_message_t *msg)
{
	char na[51], nb[51];

	if (client == NULL || client->nick == NULL
			|| msg->source.type != IRC_MESSAGE_SOURCE_USER
			|| msg->source.user.nick == NULL)
		return 0;

	mowgli_strlcpy(na, client->nick->str, 51);
	mowgli_strlcpy(nb, msg->source.user.nick, 51);

	if (client->casemap != NULL) {
		client->casemap(na);
		client->casemap(nb);
	}

	return strcmp(na, nb) == 0;
}


int irc_client_process_names(irc_client_t *client, irc_message_t *msg)
{
	char *chan, *names, *p, *save;

	if (msg->args.count != 4)
		return -1;

	chan = msg->args.head->next->next->data;
	names = msg->args.tail->data;

	irc_log_info("client: processing name list for %s\n", chan);

	p = strtok_r(names, " ", &save);
	while (p != NULL) {
		irc_client_peer_join(client, p, chan);
		p = strtok_r(NULL, " ", &save);
	}

	return 0;
}

int irc_client_process_message_server(irc_client_t *client, irc_message_t *msg)
{
	char *my_nick = NULL;
	int numeric = 0;
	char *first_num_arg = NULL;
	char *last_arg = NULL;
	
	if (msg->args.count > 1)
		first_num_arg = (char*)msg->args.head->next->data;
	if (msg->args.count > 0)
		last_arg = (char*)msg->args.tail->data;

	if (strlen(msg->command) == 3 && isdigit(msg->command[0]) &&
				isdigit(msg->command[1]) && isdigit(msg->command[2])) {
		// snag the nickname from numerics
		my_nick = (char*)msg->args.head->data;
		numeric = atoi(msg->command);
	}


	if (client->isupport != NULL && numeric == 5) {
		irc_isupport_parse(client->isupport, msg);

		if (client->isupport->casemapping != IRC_ISUPPORT_CASEMAPPING_UNKNOWN)
			irc_client_set_casemapping(client, client->isupport->casemapping);
	}


	if (numeric == 331) {
		irc_client_set_topic(client, first_num_arg, "");
	} else if (numeric == 332) {
		irc_client_set_topic(client, first_num_arg, last_arg);
	} else if (numeric == 353) {
		irc_client_process_names(client, msg);
	}
		
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
	} else if (!strcmp(msg->command, "KICK")) {
		return irc_client_do_part(client, first_arg);
	} else if (!strcmp(msg->command, "QUIT")) {
		return irc_client_do_quit(client);
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
	} else if (!strcmp(msg->command, "KICK")) {
		return irc_client_peer_part(client, peer, first_arg);
	} else if (!strcmp(msg->command, "QUIT")) {
		return irc_client_peer_quit(client, peer);
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
		if (irc_client_message_is_me(client, msg)) {
			return irc_client_process_message_self(client, msg);
		} else {
			return irc_client_process_message_peer(client, msg);
		}
		break;
	}

	return 0;
}

