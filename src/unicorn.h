#ifndef __INC_UNICORN_H__
#define __INC_UNICORN_H__


#include <stddef.h>
#include <mowgli.h>


/*
 * NOTE: the formatter functions (irc_sender_format and
 * irc_message_format) ask for a buffer size argument. It wouldn't be
 * nice of us to write to more memory than you want us to. strlen(buf)
 * will always end up being less than n. This is because the formatters
 * will never write a null byte past the end of the buffer, but will
 * always be sure to terminate buf with a null byte.
 */

/* src/sender.c */

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

extern int irc_sender_parse(irc_sender_t *sender, char *spec);
extern int irc_sender_format(irc_sender_t *sender, char *buf, size_t n);


/* src/message.c */

struct irc_message_ {
        /* optional sender */
        irc_sender_t sender;

        /* mandatory command; numerics remain ASCII-encoded */
        char *command;

        /* arguments */
        mowgli_list_t args;
};
typedef struct irc_message_ irc_message_t;

extern int irc_message_parse(irc_message_t *msg, char *spec);
extern int irc_message_format(irc_message_t *msg, char *buf, size_t n);


/* src/mode.c */

struct irc_mode_ops_ {
        int (*clear)(struct irc_mode_ops_ *ops, char mode);
        int (*set)(struct irc_mode_ops_ *ops, char mode);
        int (*clear_arg)(struct irc_mode_ops_ *ops, char mode, char *arg);
        int (*set_arg)(struct irc_mode_ops_ *ops, char mode, char *arg);
        void *privdata;
};
typedef struct irc_mode_ops_ irc_mode_ops_t;

extern int irc_mode_parse(irc_mode_ops_t *ops, char *modespec, char *argmodes, mowgli_node_t *args);


/* src/nick.c */

extern void irc_nick_canonize_null(char *nick);
extern void irc_nick_canonize_toupper(char *nick);
extern void irc_nick_canonize_rfc1459(char *nick);
extern void irc_nick_canonize_strict_rfc1459(char *nick);


/* src/isupport.c */

#define IRC_ISUPPORT_CASEMAPPING_ASCII 0
#define IRC_ISUPPORT_CASEMAPPING_RFC1459 1
#define IRC_ISUPPORT_CASEMAPPING_STRICT_RFC1459 2

struct irc_isupport_ {
        int casemapping;

        struct {
                char *list;
                char *arg_always;
                char *arg_onset;
                char *noarg;
                size_t n;
        } chanmodes;

        char *chantypes;
        size_t chantypeslen;

        int modes;

        int nicklen;

        struct {
                char *mode;
                char *prefix;
                size_t n;
        } prefix;
};
typedef struct irc_isupport_ irc_isupport_t;

// This function strncpy's to non-NULL string types
// The args in msg are not left intact
extern int irc_isupport_parse(irc_isupport_t *isupport, irc_message_t *msg);

#endif
