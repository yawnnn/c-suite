#include "llist.h"

#include <stdlib.h>

/**
 * @brief create new node
 * 
 * @param[in] data node's data
 * @return node created
 */
static LLNode *llnode_new(void *data) {
   LLNode *node;

   node = malloc(sizeof(LLNode));
   node->data = data;
   node->next = NULL;

   return node;
}

/**
 * @brief free the node
 * 
 * @param[in,out] node linked list node
 * @param func_free callback to free the nodes's data
 */
static inline void llnode_free(LLNode *node, Func_Free func_free) {
   if (func_free) func_free(node->data);
   free(node);
}

void llist_init(LList *list) {
   list->head = NULL;
   list->tail = NULL;
}

void llist_free(LList *list, Func_Free func_free) {
   if (!llist_is_empty(list)) {
      LLNode *curr, *next;

      curr = list->head;
      next = curr->next;

      while (next) {
         curr = next;
         next = next->next;
         llnode_free(curr, func_free);
      }

      llnode_free(list->head, func_free);
      list->head = list->tail = NULL;
   }
}

LLNode *llist_next(LList *list, LLNode *curr) {
   if (curr)
      return curr->next;
   else
      return list->head;
}

LLNode *llist_prev(LList *list, LLNode *curr) {
   LLNode *prev;

   if (curr) {
      prev = NULL;
      while ((prev = llist_next(list, prev)) != NULL && prev->next != curr)
         ;
   } else
      prev = list->tail;

   return prev;
}

LLNode *llist_push_back(LList *list, void *data) {
   LLNode *node;

   node = llnode_new(data);

   if (!llist_is_empty(list)) {
      list->tail->next = node;
      list->tail = node;
   } else
      list->head = list->tail = node;

   return node;
}

LLNode *llist_push_front(LList *list, void *data) {
   LLNode *node;

   node = llnode_new(data);

   if (!llist_is_empty(list)) {
      node->next = list->head;
      list->head = node;
   } else
      list->head = list->tail = node;

   return node;
}

LLNode *llist_insert(LList *list, LLNode *prev, void *data) {
   LLNode *node;

   if (prev) {
      node = llnode_new(data);
      node->next = prev->next;
      prev->next = node;

      if (prev == list->tail) list->tail = node;
   } else
      node = llist_push_front(list, data);

   return node;
}

void *llist_pop_back(LList *list) {
   LLNode *to_remove;
   void *data;

   if (llist_is_empty(list)) return NULL;

   to_remove = list->tail;
   data = to_remove->data;

   if (list->head == list->tail)
      list->head = list->tail = NULL;
   else {
      LLNode *prev;

      if ((prev = llist_prev(list, list->tail)) != NULL) {
         prev->next = NULL;
         list->tail = prev;
      }
   }

   llnode_free(to_remove, NULL);

   return data;
}

void *llist_pop_front(LList *list) {
   LLNode *to_remove;
   void *data;

   if (llist_is_empty(list)) return NULL;

   to_remove = list->head;
   data = to_remove->data;

   if (list->head == list->tail)
      list->head = list->tail = NULL;
   else
      list->head = list->head->next;

   llnode_free(to_remove, NULL);

   return data;
}

void *llist_remove(LList *list, LLNode *node) {
   LLNode *prev;
   void *data;

   if (llist_is_empty(list) || !node) return NULL;

   if (node == list->head) {
      data = node->data;
      list->head = node->next;
      llnode_free(node, NULL);
   } else if ((prev = llist_prev(list, node)) != NULL) {
      data = node->data;
      prev->next = node->next;

      if (node == list->tail) list->tail = prev;

      llnode_free(node, NULL);
   } else
      data = NULL;

   return data;
}