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
 * @brief intrusive doubly linked list
 * 
 * essentially list_head from the linux kernel
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
 * @brief the corresponding data of the current element
 * 
 * @param node node
 * @param etype type of the entry
 * @param MEMBER name of the node variable inside the @p etype struct
 */
#define ilist_entry(list, etype, MEMBER)       container_of(list, etype, MEMBER)

/**
 * @brief the corresponding node from the entry
 * 
 * @param entry entry
 * @param MEMBER name of the node variable inside the @p entry struct
 */
#define ilist_from_entry(entry, MEMBER)        (&entry->MEMBER)

/**
 * @brief first entry of the list
 */
#define ilist_first_entry(list, etype, MEMBER) ilist_entry(ilist_first(list), etype, MEMBER)

/**
 * @brief last entry of the list
 */
#define ilist_last_entry(list, etype, MEMBER)  ilist_entry(ilist_last(list), etype, MEMBER)

/**
 * @brief next entry of the list
 */
#define ilist_next_entry(list, curr, etype, MEMBER) \
   ilist_entry(ilist_next(list, curr), etype, MEMBER)

/**
 * @brief previous entry of the list
 */
#define ilist_prev_entry(list, curr, etype, MEMBER) \
   ilist_entry(ilist_prev(list, curr), etype, MEMBER)

/**
 * @brief initialize list
 */
INLINE static void ilist_init(IList *list)
{
   list->prev = list->next = list;
}

/**
 * @brief insert @p node after @p at
 */
void ilist_push_front(INode *at, INode *node);

/**
 * @brief insert @p node before @p at
 */
void ilist_push_back(INode *at, INode *node);

/**
 * @brief remove @p node from the list
 */
void ilist_remove(INode *node);

/**
 * @brief if the list is empty
 */
bool ilist_is_empty(IList *list);

/**
 * @brief insert @p node after @p at
 */
void ilist_split(IList *list, IList *other, INode *at);

/**
 * @brief insert @p node after @p at
 */
void ilist_join(IList *list, IList *other);

/**
 * @brief if @p node is the end of the @p list
 * 
 * this works both for forward and reverse iteration
 */
INLINE static bool ilist_is_end(INode *node, IList *list)
{
   return node == list;
}

/**
 * @brief first node of the list
 */
INLINE static IList *ilist_first(IList *list)
{
   if (ilist_is_end(list, list->next))
      return NULL;
   return list->next;
}

/**
 * @brief last node of the list
 */
INLINE static IList *ilist_last(IList *list)
{
   if (ilist_is_end(list, list->prev))
      return NULL;
   return list->prev;
}

/**
 * @brief node after @p curr
 */
INLINE static INode *ilist_next(IList *list, INode *curr)
{
   if (ilist_is_end(list, curr))
      return NULL;
   return curr->next;
}

/**
 * @brief node before @p curr
 */
INLINE static INode *ilist_prev(IList *list, INode *curr)
{
   if (ilist_is_end(list, curr))
      return NULL;
   return curr->prev;
}

/**
 * @brief if list is empty
 */
INLINE static bool ilist_is_empty(IList *list)
{
   return ilist_first(list) == NULL;
}

#define ilist_foreach_entry(etype, ENAME, list, MEMBER) \
   for (etype *ENAME = ilist_first_entry(list, etype, MEMBER); \
        !ilist_is_end(ilist_from_entry(ENAME, MEMBER), (list)); \
        ENAME = ilist_next_entry(list, ENAME, etype, MEMBER))

#define ilist_foreach_entry_rev(etype, ENAME, list, member) \
   for (etype *ENAME = ilist_last_entry(list, etype, MEMBER); \
        !ilist_is_end(ilist_from_entry(ENAME, MEMBER), (list)); \
        ENAME = ilist_prev_entry(list, ENAME, etype, MEMBER))

#endif /* __ILIST_H__ */