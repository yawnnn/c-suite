/*
 * Copyright (c) 2026 Alessandro Martone
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stddef.h>

#include "llist.h"

/**
 * @brief insert @p node between known consecutive @p prev and @p next
 * 
 * @param[in] node new node
 * @param[in,out] prev node coming before @p next
 * @param[in,out] next node coming after @p prev
 */
INLINE static void llist_insert_between(LNode *node, LNode *prev, LNode *next)
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
INLINE static void llist_remove_between(LNode *prev, LNode *next)
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
INLINE static void llist_take_place(LNode *old, LNode *new)
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
INLINE static void llist_from_section(LList *dst, LNode *beg, LNode *end)
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
INLINE static void llist_join_between(const LList *list, LNode *prev, LNode *next)
{
   LNode *first = list->next;
   LNode *last = list->prev;

   first->prev = prev;
   prev->next = first;

   last->next = next;
   next->prev = last;
}

void llist_insert_front(LNode *node, LNode *at)
{
   llist_insert_between(node, at, at->next);
}

void llist_insert_back(LNode *node, LNode *at)
{
   llist_insert_between(node, at->prev, at);
}

void llist_remove(LNode *node)
{
   llist_remove_between(node->prev, node->next);
   llist_init(node);
}

void llist_pop_front(LList *list)
{
   if (!llist_iter_end(list, list->next))
      llist_remove(list->next);
}

void llist_pop_back(LList *list)
{
   if (!llist_iter_end(list, list->prev))
      llist_remove(list->prev);
}

LNode *llist_get(LList *list, size_t index)
{
   size_t pos = 0;
   LNode *curr;

   for (curr = (LNode *)list; (curr = llist_next(list, curr)) != NULL;) {
      if (pos++ == index)
         return curr;
   }

   return NULL;
}

void llist_swap(LNode *node1, LNode *node2)
{
   LNode *pos = node2->prev;

   llist_remove(node2);
   llist_take_place(node1, node2);
   llist_insert_front(node1, pos == node1 ? node2 : pos);
}

void llist_split_front(LList *dst, LList *src, LNode *at)
{
   if (at->next == (LNode *)src) {
      llist_init(dst);
      return;
   }

   llist_from_section(dst, at->next, (LNode *)src);
   src->prev = at;
   at->next = src;
}

void llist_split_back(LList *dst, LList *src, LNode *at)
{
   if (src->next == (LNode *)at) {
      llist_init(dst);
      return;
   }

   llist_from_section(dst, src->next, at);
   src->next = at;
   at->prev = src;
}

void llist_join_front(LList *list, LNode *at)
{
   if (!llist_is_empty(list)) {
      llist_join_between(list, at, at->next);
      llist_init(list);
   }
}

void llist_join_back(LList *list, LNode *at)
{
   if (!llist_is_empty(list)) {
      llist_join_between(list, at->prev, at);
      llist_init(list);
   }
}

size_t llist_len(const LList *list)
{
   size_t len = 0;
   LNode *curr;

   for (curr = (LNode *)list; (curr = llist_next(list, curr)) != NULL;) {
      len++;
   }

   return len;
}