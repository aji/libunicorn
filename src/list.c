#include <stdlib.h>
#include "unicorn.h"


irc_node_t *irc_node_create()
{
        irc_node_t *node;

        node = malloc(sizeof(*node));
        if (node == NULL)
                return NULL;

        node->next = node->prev = node->data = NULL;

        return node;
}

int irc_node_add(void *data, irc_node_t *node, irc_list_t *list)
{
        if (node == NULL || list == NULL)
                return -1;

        node->next = node->prev = NULL;
        node->data = data;

        if (list->head == NULL) {
                list->head = node;
                list->tail = node;
                list->count = 1;
                return 0;
        }

        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
        list->count++;

        return 0;
}

int irc_node_delete(irc_node_t *node, irc_list_t *list)
{
        if (node == NULL || list == NULL)
                return -1;

        if (node->next)
                node->next->prev = node->prev;
        if (node->prev)
                node->prev->next = node->next;

        if (list->tail == node)
                list->tail = node->prev;
        if (list->head == node)
                list->head = node->next;

        list->count--;

        return 0;
}

