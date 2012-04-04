#include <string.h>
#include <stdio.h>

#include "unicorn.h"

int irc_sender_parse(irc_sender_t *sender, char *spec)
{
        char *at;

        // clear the structure to zeros to prevent any weirdness
        memset(sender, 0, sizeof(*sender));

        // skip over any : that may have snuck in
        if (*spec == ':')
                spec++;

        // if the string is empty, then we're done here
        if (*spec == '\0') {
                sender->type = IRC_SENDER_NONE;
                return 0;
        }

        // assume the start of the string points to the nick
        sender->user.nick = spec;

        // snag ourselves the ident
        at = strchr(spec, '!');
        if (at != NULL) {
                *at++ = '\0';
                sender->user.ident = at;
                spec = at;
        }

        // and then the host
        at = strchr(spec, '@');
        if (at != NULL) {
                *at++ = '\0';
                sender->user.host = at;
        }

        // and now we determine if the nick is actually a server name
        at = strchr(sender->user.nick, '.');
        if (at != NULL && sender->user.ident == NULL && sender->user.host == NULL)
                sender->type = IRC_SENDER_SERVER;
        else
                sender->type = IRC_SENDER_USER;

        return 0;
}

int irc_sender_format(irc_sender_t *sender, mowgli_string_t *str)
{
        return_val_if_fail(str != NULL, -1);

        switch (sender->type) {
        case IRC_SENDER_NONE:
                return 0;

        case IRC_SENDER_SERVER:
                str->append(str, sender->server.name, strlen(sender->server.name));
                return 0;

        case IRC_SENDER_USER:
                str->append(str, sender->user.nick, strlen(sender->user.nick));

                if (sender->user.ident) {
                        str->append_char(str, '!');
                        str->append(str, sender->user.ident, strlen(sender->user.ident));
                }
                if (sender->user.host) {
                        str->append_char(str, '@');
                        str->append(str, sender->user.host, strlen(sender->user.host));
                }

                return 0;
        }

        // ummmm...
        return -1;
}
