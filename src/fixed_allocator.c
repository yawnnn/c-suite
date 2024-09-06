#include "fixed_allocator.h"

#include <stdlib.h>

void fixed_allocator_init(
   FixedAllocator *fixed_alloc,
   void *buffer,
   size_t size
) {
   size_t aligned_head;

   // aligning the addres to a multiple of sizeof(void *)
   aligned_head = (size_t)buffer;
   aligned_head = (aligned_head + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

   fixed_alloc->start = (char *)(aligned_head);
   fixed_alloc->head = (char *)(aligned_head);
   fixed_alloc->end = fixed_alloc->start + size;
}

void *fixed_allocator_alloc(FixedAllocator *fixed_alloc, size_t size) {
   void *next_head;

   next_head = fixed_alloc->head + size;

   if (next_head > fixed_alloc->end) return NULL;

   void *allocation;
   size_t aligned_head;

   allocation = fixed_alloc->head;
   fixed_alloc->head = next_head;

   // aligning the addres to a multiple of sizeof(void *)
   aligned_head = (size_t)fixed_alloc->head;
   aligned_head = (aligned_head + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
   fixed_alloc->head = (char *)(aligned_head);

   return allocation;
}

void *fixed_allocator_realloc(
   FixedAllocator *fixed_alloc,
   void *ptr,
   size_t new_size
) {
   // wastes the previous memory
   return fixed_allocator_alloc(fixed_alloc, new_size);
}

void fixed_allocator_clear(FixedAllocator *fixed_alloc) {
   fixed_alloc->head = fixed_alloc->start;
}