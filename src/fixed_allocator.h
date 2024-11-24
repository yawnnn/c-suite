#ifndef __FIXED_ALLOCATOR_H__
#define __FIXED_ALLOCATOR_H__

#include <stdint.h>

/**
 * @brief allocates memory only within the initial buffer provided to init
 */
typedef struct FixedAllocator {
   unsigned char *start;
   unsigned char *end;
   unsigned char *head;
} FixedAllocator;

void  fixed_allocator_init(FixedAllocator *allocator, void *buffer, size_t size);
void *fixed_allocator_alloc(FixedAllocator *allocator, size_t size);
void *fixed_allocator_realloc(FixedAllocator *allocator, size_t size, void *prev_allocation);
void  fixed_allocator_free(FixedAllocator *allocator, void *ptr);
void  fixed_allocator_deinit(FixedAllocator *allocator);

#endif /* __FIXED_ALLOCATOR_H__ */