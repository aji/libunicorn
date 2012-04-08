#ifndef __INC_UNICORN_H__
#define __INC_UNICORN_H__


#include <stddef.h>
#include <mowgli.h>


/* src/message.c */

#define IRC_MESSAGE_SOURCE_NONE   0
#define IRC_MESSAGE_SOURCE_SERVER 1
#define IRC_MESSAGE_SOURCE_USER   2

struct irc_message_ {
	/* 512-byte character buffer */
	char buffer[512];

	/* potential source */
	union irc_message_source_ {
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
typedef union irc_message_source_ irc_message_source_t;
typedef struct irc_message_ irc_message_t;

extern int irc_message_source_parse(irc_message_source_t *source, char *spec);
extern int irc_message_source_format(irc_message_source_t *source, mowgli_string_t *str);

extern int irc_message_parse_buffer(irc_message_t *msg);
extern int irc_message_parse(irc_message_t *msg, const char *spec);
extern int irc_message_format(irc_message_t *msg, mowgli_string_t *str);


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
extern void irc_nick_canonize_ascii(char *nick);
extern void irc_nick_canonize_rfc1459(char *nick);
extern void irc_nick_canonize_strict_rfc1459(char *nick);


/* src/isupport.c */

#define IRC_ISUPPORT_CASEMAPPING_ASCII 0
#define IRC_ISUPPORT_CASEMAPPING_RFC1459 1
#define IRC_ISUPPORT_CASEMAPPING_STRICT_RFC1459 2

struct irc_isupport_ {
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
typedef struct irc_isupport_ irc_isupport_t;

// This function strncpy's to non-NULL string types
// The args in msg are not left intact
extern int irc_isupport_parse(irc_isupport_t *isupport, irc_message_t *msg);

#endif
