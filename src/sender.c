#include <string.h>
#include <stdio.h>
#include <unicorn.h>

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

int irc_sender_format(irc_sender_t *sender, char *buf, size_t n)
{
        if (n == 0)
                return -1;

        switch (sender->type) {
        case IRC_SENDER_NONE:
                buf[0] = '\0';
                return 0;

        case IRC_SENDER_SERVER:
                strncpy(buf, sender->server.name, n);
                return 0;

        case IRC_SENDER_USER:
                strncpy(buf, sender->user.nick, n);

                if (sender->user.ident) {
                        strcat(buf, "!");
                        strncat(buf, sender->user.ident, n - strlen(buf));
                }
                if (sender->user.host) {
                        strcat(buf, "@");
                        strncat(buf, sender->user.host, n - strlen(buf));
                }

                return 0;
        }

        // ummmm...
        return -1;
}
