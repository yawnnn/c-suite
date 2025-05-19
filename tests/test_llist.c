#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llist.h"

void dbg_front(LList *list) {
   printf("\n### FORWARDS ###\n");

   bool first = true;
   bool found_tail = false;
   for (LLNode *next = NULL; (next = llist_next(list, next)) != NULL;) {
      printf("%s%d", first ? "" : " -> ", *(int *)next->data);
      first = false;

      if (next == list->tail)
         found_tail = true;
   }
   if (!first)
      printf("\n");
   printf("found_tail: %d\n", (int)found_tail);
}

void dbg_back(LList *list) {
   printf("\n### BACKWARDS ###\n");

   bool first = true;
   bool found_head = false;
   for (LLNode *next = NULL; (next = llist_prev(list, next)) != NULL;) {
      printf("%s%d", first ? "" : " -> ", *(int *)next->data);
      first = false;

      if (next == list->tail)
         found_head = true;
   }
   if (!first)
      printf("\n");
   printf("found_head: %d\n", (int)found_head);
}

int *get_unique_id() {
   static int s_global_counter = 0;
   int       *id = (int *)malloc(sizeof(int));
   *id = ++s_global_counter;
   return id;
}

int main() {
   LList list = {0};

   LLNode *n1 = llist_push_back(&list, get_unique_id());
   int    *v1;
   LLNode *n2 = llist_push_back(&list, get_unique_id());
   int    *v2;
   LLNode *n3 = llist_push_front(&list, get_unique_id());
   int    *v3;
   /*LLNode *n4 = */ llist_push_front(&list, get_unique_id());
   int *v4;
   /*LLNode *n5 = */ llist_push_back(&list, get_unique_id());
   int *v5;

   dbg_front(&list);  //       dbg_back(&list);

   v4 = llist_pop_front(&list);
   free(v4);
   v5 = llist_pop_back(&list);
   free(v5);

   dbg_front(&list);  //       dbg_back(&list);

   /*LLNode *n6 = */ llist_insert(&list, llist_prev(&list, n2), get_unique_id());
   int    *v6;
   LLNode *n7 = llist_insert(&list, llist_next(&list, n3), get_unique_id());
   int    *v7;
   /*LLNode *n8 = */ llist_insert(&list, llist_prev(&list, n1), get_unique_id());
   int *v8;

   dbg_front(&list);
   dbg_back(&list);

   v1 = llist_remove(&list, n1);
   free(v1);
   v7 = llist_remove(&list, n7);
   free(v7);
   v6 = llist_remove(&list, llist_prev(&list, n2));
   free(v6);
   v8 = llist_remove(&list, llist_prev(&list, n2));
   free(v8);

   /*LLNode *n9 = */ llist_insert(&list, llist_prev(&list, n2), get_unique_id());
   int *v9;

   v2 = llist_remove(&list, list.tail);
   free(v2);
   v3 = llist_remove(&list, list.head);
   free(v3);
   //v9 = llist_remove(&list, list.head);      free(v9);
   v9 = llist_remove(&list, list.tail);
   free(v9);
   //v__ = llist_remove(&list, n__);      free(v__);

   // v3 = llist_remove(&list, n3);      free(v3);
   // v8 = llist_remove(&list, n8);      free(v8);
   // v1 = llist_remove(&list, n1);      free(v1);
   // v7 = llist_remove(&list, n7);      free(v7);
   // v6 = llist_remove(&list, n6);      free(v6);
   // v2 = llist_remove(&list, n2);      free(v2);
}