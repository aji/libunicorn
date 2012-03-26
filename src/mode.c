#include <string.h>
#include "unicorn.h"

int irc_mode_parse(irc_mode_ops_t *ops, char *modespec, char *argmodes, irc_node_t *arghead)
{
        int error = 0;
        int (*cb)(irc_mode_ops_t*, char) = NULL;
        int (*cb_arg)(irc_mode_ops_t*, char, char*) = NULL;
        char m;

        while (*modespec) {
                m = *modespec++;

                if (m == '-') {
                        cb = ops->clear;
                        cb_arg = ops->clear_arg;
                        continue;
                } else if (m == '+') {
                        cb = ops->set;
                        cb_arg = ops->set_arg;
                        continue;
                }

                if (strchr(argmodes, m)) {
                        if (arghead == NULL || cb_arg == NULL) {
                                error = -1;
                        } else {
                                cb_arg(ops, m, arghead->data);
                                arghead = arghead->next;
                        }
                } else {
                        if (cb == NULL)
                                error = -1;
                        else
                                cb(ops, m);
                }
        }

        return error;
}
