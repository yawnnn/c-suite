/**
 * @file ilist.h
 */
#ifndef __ILIST_H__
#define __ILIST_H__

#include <stdbool.h>
#include <stddef.h>

#if defined(_MSC_VER)
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

typedef struct INode {
   struct INode *prev;
   struct INode *next;
} INode;

/**
 * @brief intrusive circulaar doubly linked list
 * 
 * linked lists are well known and simple but rarely useful. a simple vector or more specialized struct is usually preferable.
 * intrusive lists can make sense, but especially in C are more complex and require some preprocessor stuff
 * 
 * intrusive means that you will have to put a @p INode inside the struct definition of the type you want to use it with
 * you might want to check out the tests/ folder for some examples, altough they're just meant to be tests, not particularly explainatory
 * 
 * the _front/_next methods are good for stacks and the _back/_prev methods good for queues
 * 
 * typedef used to represent the "real list" (or "first node"), which isn't part of a struct
 * 
 * implementation based on list_head from the linux kernel
 */
typedef INode IList;

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
INLINE static void ilist_init(IList *list)
{
   list->prev = list->next = list;
}

/**
 * @brief insert @p node after @p at
 * 
 * @param[in,out] node new node
 * @param[in,out] at position before insertion
 */
void ilist_insert_front(INode *node, INode *at);

/**
 * @brief insert @p node before @p at
 * 
 * @param[in,out] node new node
 * @param[in,out] at position after insertion
 */
void ilist_insert_back(INode *node, INode *at);

/**
 * @brief insert @p node at the beginning of the list
 * 
 * @param[in,out] list linked list
 * @param[in,out] node new node
 */
INLINE static void ilist_push_front(IList *list, INode *node)
{
   ilist_insert_front(node, (INode *)list);
}

/**
 * @brief insert @p node at the end of the list
 * 
 * @param[in,out] list linked list
 * @param[in,out] node new node
 */
INLINE static void ilist_push_back(IList *list, INode *node)
{
   ilist_insert_back(node, (INode *)list);
}

/**
 * @brief remove @p node from the list
 * 
 * @param[in,out] node node to be removed from the list it belongs to
 */
void ilist_remove(INode *node);

/**
 * @brief remove node from the beginning of the list
 *
 * @param[in,out] list linked list
 */
void ilist_pop_back(IList *list);

/**
 * @brief remove node from the end of the list
 *
 * @param[in,out] list linked list
 */
void ilist_pop_back(IList *list);

/**
 * @brief get node at @p index
 *
 * @param[in] list linked list
 * @param[in] index index of the node
 * 
 * @return node or NULL
 */
INode *ilist_get(IList *list, size_t index);

/**
 * @brief swap nodes
 * 
 * @param[in,out] node1 first node
 * @param[in,out] node2 second node
 */
void ilist_swap(INode *node1, INode *node2);

/**
 * @brief split @p src into two, starting after @p at until the end
 * 
 * @param[out] dst destination list
 * @param[in,out] src source list
 * @param[in,out] at position to start at (exclusive)
 */
void ilist_split_front(IList *dst, IList *src, INode *at);

/**
 * @brief split @p src into two, starting at the beginning until before @p at
 * 
 * @param[out] dst destination list
 * @param[in,out] src source list
 * @param[in,out] at position to end at (exclusive)
 */
void ilist_split_back(IList *dst, IList *src, INode *at);

/**
 * @brief insert @p list after @p at
 * 
 * @param[in,out] list list to be inserted
 * @param[in,out] at position in the other list to insert at
 */
void ilist_join_front(IList *list, INode *at);

/**
 * @brief insert @p list before @p at
 * 
 * @param[in,out] list list to be inserted
 * @param[in,out] at position in the other list to insert at
 */
void ilist_join_back(IList *list, INode *at);

/**
 * @brief if @p node represents is the member after the last/before the first of @p list
 * 
 * @param[in] list linked list
 * @param[in] node node
 */
INLINE static bool ilist_iter_end(const IList *list, const INode *node)
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
INLINE static IList *ilist_first(IList *list)
{
   if (ilist_iter_end(list, list->next))
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
INLINE static IList *ilist_last(IList *list)
{
   if (ilist_iter_end(list, list->prev))
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
INLINE static INode *ilist_next(const IList *list, INode *curr)
{
   if (ilist_iter_end(list, curr->next))
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
INLINE static INode *ilist_prev(const IList *list, INode *curr)
{
   if (ilist_iter_end(list, curr->prev))
      return NULL;
   return curr->prev;
}

/**
 * @brief if list is empty
 * 
 * @param[in] list
 */
INLINE static bool ilist_is_empty(const IList *list)
{
   return ilist_iter_end(list, list->next);
}

/**
 * @brief number of elements in the list
 * 
 * @param[in] list
 * 
 * @return number of elements
 */
size_t ilist_len(const IList *list);

/**
 * @brief the corresponding data of the current element
 * 
 * @param node node
 * @param etype type name of the entry
 * @param MEMBER name of the node variable inside the @p etype struct
 */
#define ilist_entry(list, etype, MEMBER) container_of(list, etype, MEMBER)

/**
 * @brief the corresponding node from the entry
 * 
 * @param entry entry
 * @param MEMBER name of the node variable inside the @p entry struct
 */
#define ilist_from_entry(entry, MEMBER)  (&entry->MEMBER)

/**
 * @brief first entry of the list
 * 
 * parallel of ilist_fist
 * see @p ilist_entry for params
 */
#define ilist_first_entry(list, etype, MEMBER) \
   (ilist_iter_end((list), (list)->next) ? ilist_entry((list)->next, etype, MEMBER) : NULL)

/**
 * @brief last entry of the list
 * 
 * parallel of ilist_last
 * see @p ilist_entry for params
 */
#define ilist_last_entry(list, etype, MEMBER) \
   (ilist_iter_end((list), (list)->prev) ? ilist_entry((list)->prev, etype, MEMBER) : NULL)

/**
 * @brief next entry of the list
 * 
 * parallel of ilist_next
 * see @p ilist_entry for params
 */
#define ilist_next_entry(list, curr, etype, MEMBER) \
   (ilist_iter_end((list), (curr)->next) ? ilist_entry((curr)->next, etype, MEMBER) : NULL)

/**
 * @brief previous entry of the list
 * 
 * parallel of ilist_prev
 * see @p ilist_entry for params
 */
#define ilist_prev_entry(list, curr, etype, MEMBER) \
   (ilist_iter_end((list), (curr)->prev) ? ilist_entry((curr)->prev, etype, MEMBER) : NULL)

#endif /* __ILIST_H__ */