#include "fixed_allocator.h"

#include <stdlib.h>

// pointer size is always good enough
#define DEFAULT_ALIGNMENT                (sizeof(void *))

// align `addr` to multiple of `alignment`
#define ALIGNED_ADDRESS(addr, alignment) (((addr) + (alignment) - 1) & ~((alignment) - 1))

void fixed_allocator_init(FixedAllocator *allocator, void *buffer, size_t size) {
   uintptr_t buffer_addr;

   buffer_addr = (uintptr_t)buffer;
   buffer_addr = ALIGNED_ADDRESS(buffer_addr, DEFAULT_ALIGNMENT);

   allocator->start = (unsigned char *)(buffer_addr);
   allocator->head = allocator->start;
   allocator->end = allocator->start + size;
}

void *fixed_allocator_alloc(FixedAllocator *allocator, size_t size) {
   void     *next_head, *allocation;
   uintptr_t head_addr;

   next_head = allocator->head + size;

   if (next_head > allocator->end)
      return NULL;

   allocation = allocator->head;
   allocator->head = next_head;

   head_addr = (uintptr_t)allocator->head;
   head_addr = ALIGNED_ADDRESS(head_addr, DEFAULT_ALIGNMENT);
   allocator->head = (unsigned char *)(head_addr);

   return allocation;
}

void *fixed_allocator_realloc(FixedAllocator *allocator, size_t size, void *prev_allocation) {
   // i can't free the previous allocation, so it goes to waste
   return fixed_allocator_alloc(allocator, size);
}

void fixed_allocator_free(FixedAllocator *allocator, void *ptr) {}

void fixed_allocator_deinit(FixedAllocator *allocator) {
   allocator->head = allocator->start;
}