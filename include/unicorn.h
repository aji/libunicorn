#ifndef __INC_UNICORN_H__
#define __INC_UNICORN_H__


#include <mowgli.h>

/*
 * NOTE: the formatter functions (irc_sender_format and
 * irc_message_format) ask for a buffer size argument. It wouldn't be
 * nice of us to write to more memory than you want us to. strlen(buf)
 * will always end up being less than n. This is because the formatters
 * will never write a null byte past the end of the buffer, but will
 * always be sure to terminate buf with a null byte.
 */

#define IRC_SENDER_NONE   0
#define IRC_SENDER_SERVER 1
#define IRC_SENDER_USER   2

union irc_sender_ {
        int type;

        struct {
                int type;
                char *name;
        } server;

        struct {
                int type;
                char *nick;
                char *ident;
                char *host;
        } user;
};
typedef union irc_sender_ irc_sender_t;

/* src/sender.c */
extern int irc_sender_parse(irc_sender_t *sender, char *spec);
extern int irc_sender_format(irc_sender_t *sender, char *buf, size_t n);


struct irc_message_ {
        /* optional sender */
        irc_sender_t sender;

        /* mandatory command; numerics remain ASCII-encoded */
        char *command;

        /* arguments */
        mowgli_list_t args;
};
typedef struct irc_message_ irc_message_t;

/* src/message.c */
extern int irc_message_parse(irc_message_t *msg, char *spec);
extern int irc_message_format(irc_message_t *msg, char *buf, size_t n);


#endif
