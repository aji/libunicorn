#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mowgli.h>

#include "unicorn.h"

int irc_message_parse_buffer(irc_message_t *msg)
{
	char *spec, *at;

	spec = msg->buffer;

        // first the sender, if any
        if (spec[0] == ':') {
                spec++;
                irc_sender_parse(&msg->sender, strtok_r(spec, " \r\n", &spec));
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
        if (msg->sender.type != IRC_SENDER_NONE) {
                str->append_char(str, ':');
                irc_sender_format(&msg->sender, str);
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

