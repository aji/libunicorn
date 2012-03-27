#include <string.h>
#include <stdlib.h>
#include "unicorn.h"

irc_dict_elem_t *irc_dict_leaf_create(char *canonized)
{
        irc_dict_elem_t *elem;

        elem = malloc(sizeof(*elem));
        if (elem == NULL)
                return NULL;

        elem->type = IRC_DICT_TYPE_LEAF;
        elem->leaf.canonized = canonized;
        elem->leaf.data = NULL;

        return elem;
}

irc_dict_elem_t *irc_dict_node_create()
{
        irc_dict_elem_t *elem;

        elem = malloc(sizeof(*elem));
        if (elem == NULL)
                return NULL;

        elem->type = IRC_DICT_TYPE_NODE;
        memset(elem->node.child, 0, sizeof(elem->node.child));
        elem->node.parent = NULL; 

        return elem;
}

void irc_dict_elem_free(irc_dict_elem_t *elem)
{
        if (elem->type == IRC_DICT_TYPE_LEAF)
                free(elem->leaf.canonized);
        free(elem);
}

unsigned irc_dict_node_index(char *canonized, int nibble)
{
        unsigned char mask = ((nibble & 1) == 0) ? 0xf0 : 0x0f;
        unsigned char shift = ((nibble & 1) == 0) ? 4 : 0;
        return (canonized[nibble >> 1] & mask) >> shift;
}

irc_dict_elem_t *irc_dict_find_elem(irc_dict_t *dict, char *canonized)
{
        int nibble, nibmax;
        unsigned index;
        irc_dict_elem_t *curr;

        curr = dict->root;

        nibmax = strlen(canonized) + 1 << 1;

        nibble = 0;
        while (curr != NULL && nibble < nibmax) {
                index = irc_dict_node_index(canonized, nibble);

                if (curr->type == IRC_DICT_TYPE_NODE) {
                        curr = curr->node.child[index];
                } else {
                        curr = NULL;
                }

                nibble++;
        }

        return curr;
}

irc_dict_elem_t *irc_dict_make_elem(irc_dict_t *dict, char *canonized)
{
        int nibble, nibmax;
        unsigned index;
        irc_dict_elem_t *curr;

        if (dict->root == NULL)
                dict->root = irc_dict_node_create();

        curr = dict->root;

        nibmax = strlen(canonized) + 1 << 1;

        nibble = 0;
        while (nibble < nibmax) {
                index = irc_dict_node_index(canonized, nibble);

                if (curr->type == IRC_DICT_TYPE_NODE) {
                        if (curr->node.child[index] == NULL) {
                                // if last nibble
                                if (nibble + 1 == nibmax) {
                                        curr->node.child[index] = irc_dict_leaf_create(canonized);
                                } else {
                                        curr->node.child[index] = irc_dict_node_create();
                                }

                                // taking advantage of how unions work :o
                                curr->node.child[index]->node.parent = curr;
                        }

                        curr = curr->node.child[index];
                } else {
                        return curr;
                }

                nibble++;
        }

        return curr;
}

int irc_dict_add(irc_dict_t *dict, const char *key, void *data)
{
        irc_dict_elem_t *elem;
        char *canonized;

        canonized = strdup(key);
        if (canonized == NULL)
                return -1;
        if (dict->canonize != NULL)
                dict->canonize(canonized);

        elem = irc_dict_make_elem(dict, canonized);

        if (elem->type != IRC_DICT_TYPE_LEAF)
                return -1;

        elem->leaf.data = data;

        return 0;
}

void *irc_dict_get(irc_dict_t *dict, const char *key)
{
        irc_dict_elem_t *elem;
        char *canonized;

        canonized = strdup(key);
        if (canonized == NULL)
                return NULL;
        if (dict->canonize != NULL)
                dict->canonize(canonized);

        elem = irc_dict_find_elem(dict, canonized);
        free(canonized);

        if (elem == NULL || elem->type != IRC_DICT_TYPE_LEAF)
                return NULL;

        return elem->leaf.data;
}

// TODO: clear empty parent nodes
int irc_dict_delete(irc_dict_t *dict, const char *key)
{
        irc_dict_elem_t *elem;
        char *canonized;

        canonized = strdup(key);
        if (canonized == NULL)
                return -1;
        if (dict->canonize != NULL)
                dict->canonize(canonized);

        elem = irc_dict_find_elem(dict, canonized);
        free(canonized);

        if (elem == NULL || elem->type != IRC_DICT_TYPE_LEAF)
                return -1;

        elem->leaf.parent->node.child[0] = NULL;
        irc_dict_elem_free(elem);

        return 0;
}
