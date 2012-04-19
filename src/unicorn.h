#ifndef __INC_UNICORN_H__
#define __INC_UNICORN_H__


#include <stddef.h>
#include <mowgli.h>


/* src/message.c */

#define IRC_MESSAGE_SOURCE_NONE   0
#define IRC_MESSAGE_SOURCE_SERVER 1
#define IRC_MESSAGE_SOURCE_USER   2

struct irc_message {
	/* 512-byte character buffer */
	char buffer[512];

	/* potential source */
	union irc_message_source {
		unsigned type;

		struct {
			unsigned type;
			char *name;
		} server;

		struct {
			unsigned type;
			char *nick;
			char *ident;
			char *host;
		} user;
	} source;

        /* mandatory command; numerics remain ASCII-encoded */
        char *command;

        /* arguments */
        mowgli_list_t args;
};
typedef union irc_message_source irc_message_source_t;
typedef struct irc_message irc_message_t;

extern int irc_message_source_parse(irc_message_source_t *source, char *spec);
extern int irc_message_source_format(irc_message_source_t *source, mowgli_string_t *str);

extern int irc_message_parse_buffer(irc_message_t *msg);
extern int irc_message_parse(irc_message_t *msg, const char *spec);
extern int irc_message_format(irc_message_t *msg, mowgli_string_t *str);


/* src/nick.c */

extern void irc_nick_canonize_null(char *nick);
extern void irc_nick_canonize_ascii(char *nick);
extern void irc_nick_canonize_rfc1459(char *nick);
extern void irc_nick_canonize_strict_rfc1459(char *nick);


/* src/isupport.c */

#define IRC_ISUPPORT_CASEMAPPING_UNKNOWN 0
#define IRC_ISUPPORT_CASEMAPPING_ASCII 1
#define IRC_ISUPPORT_CASEMAPPING_RFC1459 2
#define IRC_ISUPPORT_CASEMAPPING_STRICT_RFC1459 3

struct irc_isupport {
        int casemapping;

        struct {
                mowgli_string_t *list;
                mowgli_string_t *arg_always;
                mowgli_string_t *arg_onset;
                mowgli_string_t *noarg;
        } chanmodes;

        mowgli_string_t *chantypes;

        int modes;

        int nicklen;

        struct {
                mowgli_string_t *mode;
                mowgli_string_t *prefix;
        } prefix;
};
typedef struct irc_isupport irc_isupport_t;

// This function strncpy's to non-NULL string types
// The args in msg are not left intact
extern int irc_isupport_parse(irc_isupport_t *isupport, irc_message_t *msg);


/* src/client.c */

struct irc_client_peer {
	mowgli_string_t *nick;
	int ref;
};
struct irc_client_channel_user {
	struct irc_client_peer *peer;
	char prefix;
};
struct irc_client_channel {
	mowgli_string_t *name;
	mowgli_string_t *topic;
	mowgli_list_t *users;
};
struct irc_client {
	mowgli_string_t *nick;
	mowgli_patricia_t *peers;
	mowgli_patricia_t *channels;

	irc_isupport_t *isupport;

	int (*nick_cmp)(char*, char*);
};
typedef struct irc_client_peer irc_client_peer_t;
typedef struct irc_client_channel_user irc_client_channel_user_t;
typedef struct irc_client_channel irc_client_channel_t;
typedef struct irc_client irc_client_t;

extern int irc_client_init(irc_client_t *client);
extern irc_client_t *irc_client_create(void);
extern int irc_client_process_message(irc_client_t *client, irc_message_t *msg);


#endif
