#include "llist.h"

#include <stdlib.h>

static LLNode *llnode_new(void *data) {
    LLNode *node;

    node = malloc(sizeof(LLNode));
    node->data = data;
    node->next = NULL;

    return node;
}

static inline void llnode_free(LLNode *node, Func_Free func_free) {
    if (func_free)
        func_free(node->data);
    free(node);
}

void llist_init(LList *list) {
    list->head = NULL;
    list->tail = NULL;
}

void llist_free(LList *list, Func_Free func_free) {
    if (!llist_is_empty(list)) {
        LLNode *curr, *next;

        curr = list->head;
        next = curr->next;

        while (next) {
            curr = next;
            next = next->next;
            llnode_free(curr, func_free);
        }

        llnode_free(list->head, func_free);
        list->head = list->tail = NULL;
    }
}

LLNode *llist_next(LList *list, LLNode *curr) {
    if (curr)
        return curr->next;
    else
        return list->head;
}

LLNode *llist_prev(LList *list, LLNode *curr) {
    if (curr) {
        LLNode *prev = NULL;

        while ((prev = llist_next(list, prev)) != NULL && prev->next != curr)
            ;
        return prev;
    } else
        return list->tail;
}

LLNode *llist_push_back(LList *list, void *data) {
    LLNode *node;

    node = llnode_new(data);

    if (!llist_is_empty(list)) {
        list->tail->next = node;
        list->tail = node;
    } else
        list->head = list->tail = node;

    return node;
}

LLNode *llist_push_front(LList *list, void *data) {
    LLNode *node;

    node = llnode_new(data);

    if (!llist_is_empty(list)) {
        node->next = list->head;
        list->head = node;
    } else
        list->head = list->tail = node;

    return node;
}

LLNode *llist_insert(LList *list, LLNode *prev, void *data) {
    if (prev) {
        LLNode *node;

        node = llnode_new(data);
        node->next = prev->next;
        prev->next = node;

        return node;
    } else
        return llist_push_front(list, data);
}

void *llist_pop_back(LList *list) {
    if (!llist_is_empty(list)) {
        LLNode *to_remove;
        void *data;

        to_remove = list->tail;
        data = to_remove->data;

        if (list->head == list->tail)
            list->head = list->tail = NULL;
        else {
            LLNode *prev;

            if ((prev = llist_prev(list, list->tail)) != NULL) {
                prev->next = NULL;
                list->tail = prev;
            }
        }

        llnode_free(to_remove, NULL);

        return data;
    }

    return NULL;
}

void *llist_pop_front(LList *list) {
    if (!llist_is_empty(list)) {
        LLNode *to_remove;
        void *data;

        to_remove = list->head;
        data = to_remove->data;

        if (list->head == list->tail)
            list->head = list->tail = NULL;
        else
            list->head = list->head->next;

        llnode_free(to_remove, NULL);

        return data;
    }

    return NULL;
}

void *llist_remove(LList *list, LLNode *node) {
    if (!llist_is_empty(list) && node) {
        LLNode *prev;
        void *data;

        if (list->head == node) {
            list->head = node->next;
            data = node->data;
            llnode_free(node, NULL);
        } else if ((prev = llist_prev(list, node)) != NULL) {
            prev->next = node->next;
            data = node->data;
            llnode_free(node, NULL);
        } else
            data = NULL;

        return data;
    }

    return NULL;
}