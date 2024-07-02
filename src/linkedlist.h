/**
 * @file linkedlist.h
 */

#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <stdbool.h>

/**
 * @brief linked list's node
 */
typedef struct LLNode {
    void *data;             /**< the node's data */
    struct LLNode *next;    /**< the next node */
} LLNode;

/**
 * @brief callback to free nodes
 */
typedef void (*Func_Free)(void *);

/**
 * @brief linked list
 */
typedef struct LinkedList {
    LLNode *head;     /**< beginning of the list */
    LLNode *tail;     /**< end of the list */
} LinkedList;

/**
 * @brief initialize the list
 *
 * @param list linked list
 */
void LinkedList_init(LinkedList *list);

/**
 * @brief free the list
 * 
 * won't free the nodes's data, only the list, if @p func_free is NULL
 * 
 * @param list linked list
 * @param func_free callback to free the nodes's data
 */
void LinkedList_free(LinkedList *list, Func_Free func_free);

/**
 * @brief return the node after @p curr
 *
 * if @p curr == NULL, returns the first node
 * 
 * @param list linked list
 * @param curr current node
 * @return the next node
 */
LLNode *LinkedList_next(LinkedList *list, LLNode *curr);

/**
 * @brief return the node before @p curr
 *
 * if @p curr == NULL, returns the last node
 * 
 * @param list linked list
 * @param curr current node
 * @return the previous node
 */
LLNode *LinkedList_prev(LinkedList *list, LLNode *curr);

/**
 * @brief insert node at the end of the list
 *
 * @param list linked list
 * @param data node's data
 * @return the node inserted
 */
LLNode *LinkedList_push_back(LinkedList *list, void *data);

/**
 * @brief insert node at the beginning of the list
 *
 * @param list linked list
 * @param data node's data
 * @return the node inserted
 */
LLNode *LinkedList_push_front(LinkedList *list, void *data);

/**
 * @brief insert node after @p prev
 * 
 * if @p prev is NULL, insert at the beginning of the list
 *
 * @param list linked list
 * @param prev node before the one to be inserted
 * @param data node's data
 * @return the node inserted
 */
LLNode *LinkedList_insert(LinkedList *list, LLNode *prev, void *data);

/**
 * @brief remove node from the end of the list
 *
 * @param list linked list
 * @return the node's data
 */
void *LinkedList_pop_back(LinkedList *list);

/**
 * @brief remove node from the beginning of the list
 *
 * @param list linked list
 * @return the node's data
 */
void *LinkedList_pop_front(LinkedList *list);

/**
 * @brief remove @p node from the list
 *
 * @param list linked list
 * @return the node's data
 */
void *LinkedList_remove(LinkedList *list, LLNode *node);

/**
 * @brief if the list contains data
 *
 * @param list linked list
 * @return boolean
 */
inline bool LinkedList_is_empty(LinkedList *list) {
    return list->head == (void *)0;
}

#endif /* __LINKEDLIST_H__ */