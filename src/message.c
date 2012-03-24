#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "unicorn.h"

int irc_message_parse(irc_message_t *msg, char *spec)
{
        char *at;

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
                        irc_node_add(spec + 1, irc_node_create(), &msg->args);
                        break;
                } else {
                        irc_node_add(spec, irc_node_create(), &msg->args);
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

int irc_message_format(irc_message_t *msg, char *buf, size_t n)
{
        irc_node_t *curr;

        // clear the buffer
        if (n == 0)
                return -1;
        *buf = '\0';

        // first the sender, if any
        if (msg->sender.type != IRC_SENDER_NONE) {
                strncat(buf, ":", n - strlen(buf));
                irc_sender_format(&msg->sender, buf + 1, n - 1);
                strncat(buf, " ", n - strlen(buf));
        }

        // then the command
        if (msg->command == NULL)
                return -1;
        strncat(buf, msg->command, n - strlen(buf));

        // and finally the arguments
        irc_list_for_each(curr, &msg->args) {
                strncat(buf, " ", n - strlen(buf));

                if (curr->next == NULL) {
                        if (strchr(curr->data, ' ') != NULL
                                        || !strcmp(msg->command, "PRIVMSG")
                                        || !strcmp(msg->command, "NOTICE")) {
                                strncat(buf, ":", n - strlen(buf));
                        }
                }

                strncat(buf, curr->data, n - strlen(buf));
        }

        return 0;
}

