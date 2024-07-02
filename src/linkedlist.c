#include <stdlib.h>
#include "linkedlist.h"

static LLNode *LLNode_new(void *data) {
    LLNode *node;

    node = malloc(sizeof(LLNode));
    node->data = data;
    node->next = NULL;

    return node;
}

static inline void LLNode_free(LLNode *node, Func_Free func_free) {
    if (func_free)
        func_free(node->data);
    free(node);
}

void LinkedList_init(LinkedList *list) {
    list->head = NULL;
    list->tail = NULL;
}

void LinkedList_free(LinkedList *list, Func_Free func_free) {
    if (!LinkedList_is_empty(list)) {
        LLNode *curr, *next;

        curr = list->head;
        next = curr->next;

        while (next) {
            curr = next;
            next = next->next;
            LLNode_free(curr, func_free);
        }

        LLNode_free(list->head, func_free);
        list->head = list->tail = NULL;
    }
}

LLNode *LinkedList_next(LinkedList *list, LLNode *curr) {
    if (curr)
        return curr->next;
    else
        return list->head;
}

LLNode *LinkedList_prev(LinkedList *list, LLNode *curr) {
    if (curr) {
        LLNode *prev = NULL;

        while ((prev = LinkedList_next(list, prev)) != NULL && prev->next != curr)
            ;
        return prev;
    } else
        return list->tail;
}

LLNode *LinkedList_push_back(LinkedList *list, void *data) {
    LLNode *node;

    node = LLNode_new(data);

    if (!LinkedList_is_empty(list)) {
        list->tail->next = node;
        list->tail = node;
    } else
        list->head = list->tail = node;

    return node;
}

LLNode *LinkedList_push_front(LinkedList *list, void *data) {
    LLNode *node;

    node = LLNode_new(data);

    if (!LinkedList_is_empty(list)) {
        node->next = list->head;
        list->head = node;
    } else
        list->head = list->tail = node;

    return node;
}

LLNode *LinkedList_insert(LinkedList *list, LLNode *prev, void *data) {
    if (prev) {
        LLNode *node;

        node = LLNode_new(data);
        node->next = prev->next;
        prev->next = node;

        return node;
    } else
        return LinkedList_push_front(list, data);
}

void *LinkedList_pop_back(LinkedList *list) {
    if (!LinkedList_is_empty(list)) {
        LLNode *to_remove;
        void *data;

        to_remove = list->tail;
        data = to_remove->data;

        if (list->head == list->tail)
            list->head = list->tail = NULL;
        else {
            LLNode *prev;

            if ((prev = LinkedList_prev(list, list->tail)) != NULL) {
                prev->next = NULL;
                list->tail = prev;
            }
        }

        LLNode_free(to_remove, NULL);

        return data;
    }

    return NULL;
}

void *LinkedList_pop_front(LinkedList *list) {
    if (!LinkedList_is_empty(list)) {
        LLNode *to_remove;
        void *data;

        to_remove = list->head;
        data = to_remove->data;

        if (list->head == list->tail)
            list->head = list->tail = NULL;
        else
            list->head = list->head->next;

        LLNode_free(to_remove, NULL);

        return data;
    }

    return NULL;
}

void *LinkedList_remove(LinkedList *list, LLNode *node) {
    if (!LinkedList_is_empty(list) && node) {
        LLNode *prev;
        void *data;

        if (list->head == node) {
            list->head = node->next;
            data = node->data;
            LLNode_free(node, NULL);
        } else if ((prev = LinkedList_prev(list, node)) != NULL) {
            prev->next = node->next;
            data = node->data;
            LLNode_free(node, NULL);
        } else
            data = NULL;

        return data;
    }

    return NULL;
}