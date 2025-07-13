#include <stddef.h>

#include "ilist.h"

INLINE static void ilist_add(INode *node, INode *prev, INode *next)
{
   next->prev = node;
   node->next = next;
   node->prev = prev;
   prev->next = node;
}

INLINE static void ilist_del(INode *prev, INode *next)
{
   next->prev = prev;
   prev->next = next;
}

void ilist_push_front(INode *at, INode *node)
{
   ilist_add(node, at, at->next);
}

void ilist_push_back(INode *at, INode *node)
{
   ilist_add(node, at->prev, at);
}

void ilist_remove(INode *node)
{
   ilist_del(node->prev, node->next);
   node->prev = node->next = NULL;
}

void ilist_split(IList *list, IList *other, INode *at)
{
   other->next = at;
   other->prev = list->prev;
   list->prev = at->prev;
   if (list->next == at)
      list->next = list;
   at->prev = other;
   if (at->next == list)
      at->next = other;
}

void ilist_join(IList *list, IList *other)
{
   if (!ilist_is_empty(other)) {
      list->prev->next = other->next;
      other->next->prev = list->prev;
      list->prev = other->prev;
   }
   ilist_init(other);
}