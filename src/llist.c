#include <stdlib.h>

#include "llist.h"

static LNode *lnode_new(void *data, LNode *prev, LNode *next)
{
   LNode *node = malloc(sizeof(LNode));

   node->next = next;
   node->prev = prev;
   node->data = data;

   return node;
}

void llist_init(LList *llist)
{
   llist->head = llist->tail = NULL;
}

void llist_free(LList *llist)
{
   LNode *next;

   while (llist->head) {
      next = llist->head->next;
      free(llist->head);
      llist->head = next;
   }

   llist->head = llist->tail = NULL;
}

void llist_free_with(LList *llist, FreeFunc free_func)
{
   LNode *next;

   while (llist->head) {
      next = llist->head->next;
      free_func(llist->head->data);
      free(llist->head);
      llist->head = next;
   }

   llist->head = llist->tail = NULL;
}

LNode *llist_push_front(LList *llist, void *data)
{
   LNode *node = lnode_new(data, NULL, llist->head);

   if (llist->head)
      llist->head->prev = node;
   else
      llist->tail = node;

   llist->head = node;

   return node;
}

LNode *llist_push_back(LList *llist, void *data)
{
   LNode *node = lnode_new(data, llist->tail, NULL);

   if (llist->tail)
      llist->tail->next = node;
   else
      llist->head = node;

   llist->tail = node;

   return node;
}

void *llist_pop_front(LList *llist)
{
   if (!llist->head)
      return NULL;

   void  *data = llist->head->data;
   LNode *next = llist->head->next;

   if (next)
      next->prev = NULL;
   else
      llist->tail = NULL;

   free(llist->head);
   llist->head = next;

   return data;
}

void *llist_pop_back(LList *llist)
{
   if (!llist->tail)
      return NULL;

   void  *data = llist->tail->data;
   LNode *prev = llist->tail->prev;

   if (prev)
      prev->next = NULL;
   else
      llist->head = NULL;

   free(llist->tail);
   llist->tail = prev;

   return data;
}

void *llist_remove(LList *llist, LNode *node)
{
   void *data = node->data;

   if (node->prev)
      node->prev->next = node->next;
   else
      llist->head = node->next;

   if (node->next)
      node->next->prev = node->prev;
   else
      llist->tail = node->prev;

   free(node);

   return data;
}

void llist_split(LList *llist, LList *other, LNode *at)
{
   if (at->prev)
      at->prev->next = NULL;
   else
      llist->head = NULL;

   other->head = at;
   other->tail = llist->tail;
   llist->tail = at->prev;
}

void llist_join(LList *llist, LList *other)
{
   if (!llist->tail)
      *llist = *other;
   else if (other->head) {
      llist->tail->next = other->head;
      other->head->prev = llist->tail;
      llist->tail = other->tail;
   }
   other->head = other->tail = NULL;
}

void llist_swap(LNode *node1, LNode *node2)
{
   void *tmp = node1->data;
   node1->data = node2->data;
   node2->data = tmp;
}

LNode *llist_get_at(LList *llist, size_t index)
{
   LNode *node;
   size_t i = 0;

   for (node = llist->head; node && i < index; node = node->next)
      i++;

   return node;
}

size_t llist_len(const LList *llist)
{
   LNode *node;
   size_t len = 0;

   for (node = llist->head; node; node = node->next)
      len++;

   return len;
}