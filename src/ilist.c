#include <stddef.h>

#include "ilist.h"

/**
 * @brief insert @p node between known consecutive @p prev and @p next
 * 
 * @param[in] node new node
 * @param[in,out] prev node coming before @p next
 * @param[in,out] next node coming after @p prev
 */
INLINE static void ilist_insert_between(INode *node, INode *prev, INode *next)
{
   next->prev = node;
   node->next = next;
   node->prev = prev;
   prev->next = node;
}

/**
 * @brief delete nodes between @p prev and @p next
 * 
 * @param[in,out] prev node coming before @p next
 * @param[in,out] next node coming after @p prev
 */
INLINE static void ilist_remove_between(INode *prev, INode *next)
{
   next->prev = prev;
   prev->next = next;
}

/**
 * @brief take place of old node with new one
 *
 * if @p old was empty, it will be overwritten.
 *  
 * @param[in] old the node to be replaced
 * @param[out] new the new node to insert
 */
INLINE static void ilist_take_place(INode *old, INode *new)
{
   new->next = old->next;
   new->next->prev = new;
   new->prev = old->prev;
   new->prev->next = new;
}

/**
 * @brief initalize @p dst with the section of another list from [ @p beg to @p end )
 * 
 * @param[out] dst destination list
 * @param[in] beg beginning of the section (inclusive)
 * @param[in] end end of the section (exclusive)
 */
INLINE static void ilist_from_section(IList *dst, INode *beg, INode *end)
{
   dst->next = beg;
   dst->next->prev = dst;
   dst->prev = end->prev;
   dst->prev->next = dst;
}

/**
 * @brief insert @p list between consecutive @p prev and @p next
 * 
 * @param[in] list the list to add
 * @param[in,out] prev node coming before @p next
 * @param[in,out] next node coming after @p prev
 */
INLINE static void ilist_join_between(const IList *list, INode *prev, INode *next)
{
   INode *first = list->next;
   INode *last = list->prev;

   first->prev = prev;
   prev->next = first;

   last->next = next;
   next->prev = last;
}

void ilist_insert_front(INode *node, INode *at)
{
   ilist_insert_between(node, at, at->next);
}

void ilist_insert_back(INode *node, INode *at)
{
   ilist_insert_between(node, at->prev, at);
}

void ilist_remove(INode *node)
{
   ilist_remove_between(node->prev, node->next);
   ilist_init(node);
}

void ilist_pop_front(IList *list)
{
   if (!ilist_iter_end(list, list->next))
      ilist_remove(list->next);
}

void ilist_pop_back(IList *list)
{
   if (!ilist_iter_end(list, list->prev))
      ilist_remove(list->prev);
}

INode *ilist_get(IList *list, size_t index)
{
   size_t pos = 0;

   for (INode *curr = (INode *)list; (curr = ilist_next(list, curr)) != NULL;) {
      if (pos++ == index)
         return curr;
   }

   return NULL;
}

void ilist_swap(INode *node1, INode *node2)
{
   INode *pos = node2->prev;

   ilist_remove(node2);
   ilist_take_place(node1, node2);
   ilist_insert_front(node1, pos == node1 ? node2 : pos);
}

void ilist_split_front(IList *dst, IList *src, INode *at)
{
   if (at->next == (INode *)src) {
      ilist_init(dst);
      return;
   }

   ilist_from_section(dst, at->next, (INode *)src);
   src->prev = at;
   at->next = src;
}

void ilist_split_back(IList *dst, IList *src, INode *at)
{
   if (src->next == (INode *)at) {
      ilist_init(dst);
      return;
   }

   ilist_from_section(dst, src->next, at);
   src->next = at;
   at->prev = src;
}

void ilist_join_front(IList *list, INode *at)
{
   if (!ilist_is_empty(list)) {
      ilist_join_between(list, at, at->next);
      ilist_init(list);
   }
}

void ilist_join_back(IList *list, INode *at)
{
   if (!ilist_is_empty(list)) {
      ilist_join_between(list, at->prev, at);
      ilist_init(list);
   }
}

size_t ilist_len(const IList *list)
{
   size_t len = 0;

   for (INode *curr = (INode *)list; (curr = ilist_next(list, curr)) != NULL;) {
      len++;
   }

   return len;
}