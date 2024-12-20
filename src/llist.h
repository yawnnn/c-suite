/**
 * @file llist.h
 */

#ifndef __LLIST_H__
#define __LLIST_H__

#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief linked list's node
 */
typedef struct LLNode {
   void          *data; /**< the node's data */
   struct LLNode *next; /**< the next node */
} LLNode;

/**
 * @brief callback to free nodes
 */
typedef void (*Func_Free)(void *);

/**
 * @brief linked list
 */
typedef struct LList {
   LLNode *head; /**< beginning of the list */
   LLNode *tail; /**< end of the list */
} LList;

/**
 * @brief initialize the list
 *
 * @param[out] list linked list
 */
void llist_init(LList *list);

/**
 * @brief free the list
 * 
 * won't free the nodes's data, only the list, if @p func_free is NULL
 * 
 * @param[in,out] list linked list
 * @param[in] func_free callback to free the nodes's data
 */
void llist_free(LList *list, Func_Free func_free);

/**
 * @brief return the node after @p curr
 *
 * if @p curr == NULL, returns the first node
 * 
 * @param[in] list linked list
 * @param[in] curr current node
 * @return the next node
 */
LLNode *llist_next(LList *list, LLNode *curr);

/**
 * @brief return the node before @p curr
 *
 * if @p curr == NULL, returns the last node
 * 
 * @param[in] list linked list
 * @param[in] curr current node
 * @return the previous node
 */
LLNode *llist_prev(LList *list, LLNode *curr);

/**
 * @brief insert node at the end of the list
 *
 * @param[in,out] list linked list
 * @param[in] data node's data
 * @return the node inserted
 */
LLNode *llist_push_back(LList *list, void *data);

/**
 * @brief insert node at the beginning of the list
 *
 * @param[in,out] list linked list
 * @param[in] data node's data
 * @return the node inserted
 */
LLNode *llist_push_front(LList *list, void *data);

/**
 * @brief insert node after @p prev
 * 
 * if @p prev is NULL, insert at the beginning of the list
 *
 * @param[in,out] list linked list
 * @param[in] prev node before the one to be inserted
 * @param[in] data node's data
 * @return the node inserted
 */
LLNode *llist_insert(LList *list, LLNode *prev, void *data);

/**
 * @brief remove node from the end of the list
 *
 * @param[in,out] list linked list
 * @return the node's data
 */
void *llist_pop_back(LList *list);

/**
 * @brief remove node from the beginning of the list
 *
 * @param[in,out] list linked list
 * @return the node's data
 */
void *llist_pop_front(LList *list);

/**
 * @brief remove @p node from the list
 *
 * @param[in,out] list linked list
 * @return the node's data
 */
void *llist_remove(LList *list, LLNode *node);

/**
 * @brief check if list is empty
 *
 * @param[in] list linked list
 * @return boolean
 */
static inline bool llist_is_empty(LList *list) {
   return list->head == NULL;
}

#endif /* __LLIST_H__ */