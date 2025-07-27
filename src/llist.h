/**
 * @file llist.h
 */
#ifndef __LLIST_H__
#define __LLIST_H__

#include <stdbool.h>
#include <stddef.h>

#if defined(_MSC_VER)
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

typedef struct LNode {
   struct LNode *prev;
   struct LNode *next;
} LNode;

/**
 * @brief intrusive circulaar doubly linked list
 * 
 * linked lists are well known and simple but rarely useful. a simple vector or more specialized struct is usually preferable.
 * intrusive lists can make sense, but especially in C are more complex and require some preprocessor stuff
 * 
 * intrusive means that you will have to put a @p LNode inside the struct definition of the type you want to use it with
 * you might want to check out the tests/ folder for some examples, altough they're just meant to be tests, not particularly explainatory
 * 
 * the _front/_next methods are good for stacks and the _back/_prev methods good for queues
 * 
 * typedef used to represent the "real list" (or "first node"), which isn't part of a struct
 * 
 * implementation based on list_head from the linux kernel
 */
typedef LNode LList;

/**
 * @brief cast a member of a structure out to the containing structure
 * 
 * @param ptr pointer to the member
 * @param type	type of the struct that contains @p ptr
 * @param MEMBER name of the member of type typeof(*ptr) within the @p type struct
 *
 * WARNING: any const qualifier of @p ptr is lost
 * 
 * the ternary if trick ensures that the type of @p ptr is compatible with the type of &((type *)0)->MEMBER)
 * 
 * @return pointer of type @p type
 */
#define container_of(ptr, type, MEMBER) \
   ((type *)((char *)(1 ? (ptr) : &((type *)0)->MEMBER) - offsetof(type, MEMBER)))

/**
 * @brief initialize list
 * 
 * @param[out] list linked list
 */
INLINE static void llist_init(LList *list)
{
   list->prev = list->next = list;
}

/**
 * @brief insert @p node after @p at
 * 
 * @param[in,out] node new node
 * @param[in,out] at position before insertion
 */
void llist_insert_front(LNode *node, LNode *at);

/**
 * @brief insert @p node before @p at
 * 
 * @param[in,out] node new node
 * @param[in,out] at position after insertion
 */
void llist_insert_back(LNode *node, LNode *at);

/**
 * @brief insert @p node at the beginning of the list
 * 
 * @param[in,out] list linked list
 * @param[in,out] node new node
 */
INLINE static void llist_push_front(LList *list, LNode *node)
{
   llist_insert_front(node, (LNode *)list);
}

/**
 * @brief insert @p node at the end of the list
 * 
 * @param[in,out] list linked list
 * @param[in,out] node new node
 */
INLINE static void llist_push_back(LList *list, LNode *node)
{
   llist_insert_back(node, (LNode *)list);
}

/**
 * @brief remove @p node from the list
 * 
 * @param[in,out] node node to be removed from the list it belongs to
 */
void llist_remove(LNode *node);

/**
 * @brief remove node from the beginning of the list
 *
 * @param[in,out] list linked list
 */
void llist_pop_back(LList *list);

/**
 * @brief remove node from the end of the list
 *
 * @param[in,out] list linked list
 */
void llist_pop_back(LList *list);

/**
 * @brief get node at @p index
 *
 * @param[in] list linked list
 * @param[in] index index of the node
 * 
 * @return node or NULL
 */
LNode *llist_get(LList *list, size_t index);

/**
 * @brief swap nodes
 * 
 * @param[in,out] node1 first node
 * @param[in,out] node2 second node
 */
void llist_swap(LNode *node1, LNode *node2);

/**
 * @brief split @p src into two, starting after @p at until the end
 * 
 * @param[out] dst destination list
 * @param[in,out] src source list
 * @param[in,out] at position to start at (exclusive)
 */
void llist_split_front(LList *dst, LList *src, LNode *at);

/**
 * @brief split @p src into two, starting at the beginning until before @p at
 * 
 * @param[out] dst destination list
 * @param[in,out] src source list
 * @param[in,out] at position to end at (exclusive)
 */
void llist_split_back(LList *dst, LList *src, LNode *at);

/**
 * @brief insert @p list after @p at
 * 
 * @param[in,out] list list to be inserted
 * @param[in,out] at position in the other list to insert at
 */
void llist_join_front(LList *list, LNode *at);

/**
 * @brief insert @p list before @p at
 * 
 * @param[in,out] list list to be inserted
 * @param[in,out] at position in the other list to insert at
 */
void llist_join_back(LList *list, LNode *at);

/**
 * @brief if @p node represents is the member after the last/before the first of @p list
 * 
 * @param[in] list linked list
 * @param[in] node node
 */
INLINE static bool llist_iter_end(const LList *list, const LNode *node)
{
   return node == list;
}

/**
 * @brief first node of the list
 * 
 * @param[in] list linked list
 * 
 * @return first node or NULL
 */
INLINE static LList *llist_first(LList *list)
{
   if (llist_iter_end(list, list->next))
      return NULL;
   return list->next;
}

/**
 * @brief last node of the list
 * 
 * @param[in] list linked list
 * 
 * @return last node or NULL
 */
INLINE static LList *llist_last(LList *list)
{
   if (llist_iter_end(list, list->prev))
      return NULL;
   return list->prev;
}

/**
 * @brief node after @p curr
 * 
 * @param[in] list linked list
 * @param[in] curr current/starting node
 * 
 * @return node after @p curr or NULL
 */
INLINE static LNode *llist_next(const LList *list, LNode *curr)
{
   if (llist_iter_end(list, curr->next))
      return NULL;
   return curr->next;
}

/**
 * @brief node before @p curr
 * 
 * @param[in] list
 * @param[in] curr current/starting node
 * 
 * @return node before @p curr or NULL
 */
INLINE static LNode *llist_prev(const LList *list, LNode *curr)
{
   if (llist_iter_end(list, curr->prev))
      return NULL;
   return curr->prev;
}

/**
 * @brief if list is empty
 * 
 * @param[in] list
 */
INLINE static bool llist_is_empty(const LList *list)
{
   return llist_iter_end(list, list->next);
}

/**
 * @brief number of elements in the list
 * 
 * @param[in] list
 * 
 * @return number of elements
 */
size_t llist_len(const LList *list);

/**
 * @brief the corresponding data of the current element
 * 
 * @param node node
 * @param etype type name of the entry
 * @param MEMBER name of the node variable inside the @p etype struct
 */
#define llist_entry(list, etype, MEMBER) container_of(list, etype, MEMBER)

/**
 * @brief the corresponding node from the entry
 * 
 * @param entry entry
 * @param MEMBER name of the node variable inside the @p entry struct
 */
#define llist_from_entry(entry, MEMBER)  (&entry->MEMBER)

/**
 * @brief first entry of the list
 * 
 * parallel of llist_fist
 * see @p llist_entry for params
 */
#define llist_first_entry(list, etype, MEMBER) \
   (llist_iter_end((list), (list)->next) ? llist_entry((list)->next, etype, MEMBER) : NULL)

/**
 * @brief last entry of the list
 * 
 * parallel of llist_last
 * see @p llist_entry for params
 */
#define llist_last_entry(list, etype, MEMBER) \
   (llist_iter_end((list), (list)->prev) ? llist_entry((list)->prev, etype, MEMBER) : NULL)

/**
 * @brief next entry of the list
 * 
 * parallel of llist_next
 * see @p llist_entry for params
 */
#define llist_next_entry(list, curr, etype, MEMBER) \
   (llist_iter_end((list), (curr)->next) ? llist_entry((curr)->next, etype, MEMBER) : NULL)

/**
 * @brief previous entry of the list
 * 
 * parallel of llist_prev
 * see @p llist_entry for params
 */
#define llist_prev_entry(list, curr, etype, MEMBER) \
   (llist_iter_end((list), (curr)->prev) ? llist_entry((curr)->prev, etype, MEMBER) : NULL)

#endif /* __LLIST_H__ */