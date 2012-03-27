#include <ctype.h>
#include "unicorn.h"

void irc_nick_canonize_null(char *nick)
{
        return;
}

void irc_nick_canonize_rfc1459(char *nick)
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

