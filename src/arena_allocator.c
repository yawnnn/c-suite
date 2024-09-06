#include "arena_allocator.h"

#include <stdlib.h>

void arena_allocator_init(ArenaAllocator *arena_alloc) {
   arena_alloc->head = NULL;
}

void *arena_allocator_alloc(ArenaAllocator *arena_alloc, size_t bytes) {
   ArenaNode *next;

   next = (ArenaNode *)malloc(sizeof(ArenaNode) + bytes);
   next->next = arena_alloc->head;
   arena_alloc->head = next;

   /* normally you should worry about alignment here, but from what i understand
     * the result of malloc is already aligned
     * and sizeof(ArenaNode) == sizeof(ArenaNode *) == sizeof(void *), which is always a good alignment */
   return ((char *)next) + sizeof(ArenaNode);
}

void *arena_allocator_realloc(
   ArenaAllocator *arena_alloc,
   size_t bytes,
   void *prev_allocation
) {
   if (arena_alloc->head) {
      ArenaNode *curr, *prev, *needle;

      needle = (ArenaNode *)(((char *)prev_allocation) - sizeof(ArenaNode));

      prev = NULL;
      curr = arena_alloc->head;
      while (curr && curr != needle) {
         prev = curr;
         curr = curr->next;
      }

      if (curr) {
         ArenaNode *next;

         next = curr->next;
         needle = (ArenaNode *)realloc(needle, sizeof(ArenaNode) + bytes);
         needle->next = next;

         if (prev)
            prev->next = needle;
         else
            arena_alloc->head = needle;

         return ((char *)next) + sizeof(ArenaNode);
      }
   }

   return NULL;
}

void arena_allocator_free(ArenaAllocator *arena_alloc) {
   if (arena_alloc->head) {
      ArenaNode *curr, *next;

      curr = arena_alloc->head;
      next = curr->next;
      while (next) {
         curr = next;
         next = next->next;
         free(curr);
      }

      free(arena_alloc->head);
      arena_alloc->head = NULL;
   }
}