#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mowgli.h>

#include "chat.h"

int irc_message_source_parse(irc_message_source_t *source, char *spec)
{
	char *at;

	// clear the structure to zeros to prevent any weirdness
	memset(source, 0, sizeof(*source));

	// skip over any : that may have snuck in
	if (*spec == ':')
		spec++;

	// if the string is empty, then we're done here
	if (*spec == '\0') {
		source->type = IRC_MESSAGE_SOURCE_NONE;
		return 0;
	}

	// assume the start of the string points to the nick
	source->user.nick = spec;

	// snag ourselves the ident
	at = strchr(spec, '!');
	if (at != NULL) {
		*at++ = '\0';
		source->user.ident = at;
		spec = at;
	}

	// and then the host
	at = strchr(spec, '@');
	if (at != NULL) {
		*at++ = '\0';
		source->user.host = at;
	}

	// and now we determine if the nick is actually a server name
	at = strchr(source->user.nick, '.');
	if (at != NULL && source->user.ident == NULL && source->user.host == NULL)
		source->type = IRC_MESSAGE_SOURCE_SERVER;
	else
		source->type = IRC_MESSAGE_SOURCE_USER;

	return 0;
}

int irc_message_source_format(irc_message_source_t *source, mowgli_string_t *str)
{
	return_val_if_fail(str != NULL, -1);

	switch (source->type) {
	case IRC_MESSAGE_SOURCE_NONE:
		return 0;

	case IRC_MESSAGE_SOURCE_SERVER:
		str->append(str, source->server.name, strlen(source->server.name));
		return 0;

	case IRC_MESSAGE_SOURCE_USER:
		str->append(str, source->user.nick, strlen(source->user.nick));

		if (source->user.ident) {
			str->append_char(str, '!');
			str->append(str, source->user.ident, strlen(source->user.ident));
		}
		if (source->user.host) {
			str->append_char(str, '@');
			str->append(str, source->user.host, strlen(source->user.host));
		}

		return 0;
	}

	// ummmm...
	return -1;
}


int irc_message_parse_buffer(irc_message_t *msg)
{
	char *spec, *at;

	spec = msg->buffer;

	// first the sender, if any
	if (spec[0] == ':') {
		spec++;
		irc_message_source_parse(&msg->source, strtok_r(spec, " \r\n", &spec));
	}

	// then the command
	msg->command = strtok_r(spec, " \r\n", &spec);
	if (!msg->command)
		return -1;
	at = msg->command;
	while (at && *at) {
		*at = toupper(*at);
		at++;
	}

	// and now args repeatedly
	while(spec && *spec) {
		if (*spec == ':') {
			mowgli_node_add(spec + 1, mowgli_node_create(), &msg->args);
			break;
		} else {
			mowgli_node_add(spec, mowgli_node_create(), &msg->args);
			strtok_r(spec, " \r\n", &spec);
		}
	}

	// and strip any of those pesky \r\n's off the last argument
	if (msg->args.tail) {
		spec = msg->args.tail->data;
		if (at = strchr(spec, '\r'))
			*at = '\0';
		if (at = strchr(spec, '\n'))
			*at = '\0';
	}

	return 0;
}

int irc_message_parse(irc_message_t *msg, const char *spec)
{
	strncpy(msg->buffer, spec, 511);
	msg->buffer[512] = '\0';

	return irc_message_parse_buffer(msg);
}

int irc_message_format(irc_message_t *msg, mowgli_string_t *str)
{
	mowgli_node_t *curr;

	return_val_if_fail(str != NULL, -1);

	// first the sender, if any
	if (msg->source.type != IRC_MESSAGE_SOURCE_NONE) {
		str->append_char(str, ':');
		irc_message_source_format(&msg->source, str);
		str->append_char(str, ' ');
	}

	// then the command
	if (msg->command == NULL)
		return -1;
	str->append(str, msg->command, strlen(msg->command));

	// and finally the arguments
	MOWGLI_LIST_FOREACH(curr, msg->args.head) {
		str->append_char(str, ' ');

		if (curr->next == NULL) {
			if (strchr(curr->data, ' ') != NULL
					|| !strcmp(msg->command, "PRIVMSG")
					|| !strcmp(msg->command, "NOTICE")) {
				str->append_char(str, ':');
			}
		}

		str->append(str, curr->data, strlen(curr->data));
	}

	return 0;
}

