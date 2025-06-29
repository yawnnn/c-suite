/**
 * @file llist.h
 */
#ifndef __LLIST_H__
#define __LLIST_H__

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief double linked list node
 */
typedef struct LNode {
   void         *data;
   struct LNode *prev;
   struct LNode *next;
} LNode;

/**
 * @brief double linked list
 */
typedef struct LList {
   LNode *head;
   LNode *tail;
} LList;

/**
 * @brief callback to free node's data
 */
typedef void (*FreeFunc)(void *);

/**
 * @brief initialize the list (equivalent to memset)
 *
 * @param[out] llist linked list
 */
void llist_init(LList *llist);

/**
 * @brief free the list, but not the data it contains
 * 
 * WARNING: this won't free the nodes's data. see `llist_free_with`
 * 
 * @param[in,out] llist linked list
 */
void llist_free(LList *llist);

/**
 * @brief free the list and the data it contains
 * 
 * @param[in,out] llist linked list
 * @param[in] free_func callback to free the nodes's data
 */
void llist_free_with(LList *llist, FreeFunc free_func);

/**
 * @brief insert node at the beginning of the list
 *
 * @param[in,out] llist linked list
 * @param[in] data node's data
 * @return the node created
 */
LNode *llist_push_front(LList *llist, void *data);

/**
 * @brief insert node at the end of the list
 *
 * @param[in,out] llist linked list
 * @param[in] data node's data
 * @return the node created
 */
LNode *llist_push_back(LList *llist, void *data);

/**
 * @brief remove node from the beginning of the list
 *
 * @param[in,out] llist linked list
 * @return the node's data
 */
void *llist_pop_front(LList *llist);

/**
 * @brief remove node from the end of the list
 *
 * @param[in,out] llist linked list
 * @return the node's data
 */
void *llist_pop_back(LList *llist);

/**
 * @brief remove @p node from the list
 *
 * @param[in,out] llist linked list
 * @return the node's data
 */
void *llist_remove(LList *llist, LNode *node);

/**
 * @brief split the index in two
 * 
 * @param[in,out] llist original linked list. will contain the list until @p where (exclusive)
 * @param[out] other will contain the list from @p where (inclusive)
 * @param[in] where node to split at
 */
void llist_split(LList *llist, LList *other, LNode *where);

/**
 * @brief join @p other to the @p llist
 * 
 * @param[in,out] llist linked list to receive @p other
 * @param[in,out] other linked list to join to @p llist, zeroing out itself
 */
void llist_join(LList *llist, LList *other);

/**
 * @brief swap nodes
 * 
 * @param[in,out] node1 first node
 * @param[in,out] node2 second node
 */
void llist_swap(LNode *node1, LNode *node2);

/**
 * @brief get a node by index
 * 
 * @param[in] llist linked list
 * @param[in] index index
 * @return the node, or NULL
 */
LNode *llist_get_at(LList *llist, size_t index);

/**
 * @brief check if list is empty
 *
 * @param[in] llist linked list
 * @return boolean
 */
static inline bool llist_is_empty(LList *llist)
{
   return !llist->head;
}

/**
 * @brief the list's length
 *
 * @param[in] llist linked list
 * @return length
 */
size_t llist_len(const LList *llist);

#endif /* __LLIST_H__ */