#include "tracking_allocator.h"

#include <stdlib.h>

void tracking_allocator_init(TrackingAllocator *allocator) {
   TrackingList *list = &allocator->list;

   list->head = NULL;
}

void *tracking_allocator_alloc(TrackingAllocator *allocator, size_t size) {
   TrackingList *list = &allocator->list;
   TrackingNode *next;

   next = (TrackingNode *)malloc(sizeof(TrackingNode) + size);
   next->next = list->head;
   list->head = next;

   /* normally you should worry about alignment here, but from what i understand
    * the result of malloc is already aligned
    * and sizeof(TrackingNode) == sizeof(TrackingNode *) == sizeof(void *), which is always a good alignment */
   return ((char *)next) + sizeof(TrackingNode);
}

void *tracking_allocator_realloc(TrackingAllocator *allocator, size_t size, void *prev_allocation) {
   TrackingList *list = &allocator->list;

   if (list->head) {
      TrackingNode *curr, *prev, *needle;

      needle = (TrackingNode *)(((char *)prev_allocation) - sizeof(TrackingNode));

      prev = NULL;
      curr = list->head;
      while (curr && curr != needle) {
         prev = curr;
         curr = curr->next;
      }

      if (curr) {
         TrackingNode *next;

         next = curr->next;
         needle = (TrackingNode *)realloc(needle, sizeof(TrackingNode) + size);
         needle->next = next;

         if (prev)
            prev->next = needle;
         else
            list->head = needle;

         return ((char *)next) + sizeof(TrackingNode);
      }
   }

   return NULL;
}

void tracking_allocator_free(TrackingAllocator *allocator) {
   TrackingList *list = &allocator->list;

   if (list->head) {
      TrackingNode *curr, *next;

      curr = list->head;
      next = curr->next;
      while (next) {
         curr = next;
         next = next->next;
         free(curr);
      }

      free(list->head);
      list->head = NULL;
   }
}

void tracking_allocator_reset(TrackingAllocator *allocator) {}