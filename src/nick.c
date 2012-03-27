#include <ctype.h>
#include "unicorn.h"

void irc_nick_canonize_null(char *nick)
{
}

void irc_nick_canonize_toupper(char *nick)
{
        while (*nick) {
                *nick = toupper(*nick);
                nick++;
        }
}

void irc_nick_canonize_rfc1459(char *nick)
{
        while (*nick) {
                *nick = toupper(*nick);

                switch (*nick) {
                case '{': *nick = '['; break;
                case '}': *nick = ']'; break;
                case '|': *nick = '\\'; break;
                case '^': *nick = '~'; break;
                }
        }
}

void irc_nick_canonize_strict_rfc1459(char *nick)
{
        while (*nick) {
                *nick = toupper(*nick);

                switch (*nick) {
                case '{': *nick = '['; break;
                case '}': *nick = ']'; break;
                case '|': *nick = '\\'; break;
                }

                nick++;
        }
}
