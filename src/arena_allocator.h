#ifndef __ARENA_ALLOCATOR_H__
#define __ARENA_ALLOCATOR_H__

#include <stdint.h>

typedef struct Arena {
   unsigned char *start;
   unsigned char *head;
   unsigned char *end;
} Arena;

typedef struct ArenaList {
   Arena *ptr;
   size_t len;
} ArenaList;

typedef struct ArenaAllocator {
   ArenaList arena_list;
   size_t    default_arena_size;
} ArenaAllocator;

void  arena_allocator_init(ArenaAllocator *allocator, size_t default_arena_size);
void *arena_allocator_alloc(ArenaAllocator *allocator, size_t size);
void *arena_allocator_realloc(ArenaAllocator *allocator, size_t size, void *old_block);
void  arena_allocator_free(ArenaAllocator *allocator, void *block);
void  arena_allocator_deinit(ArenaAllocator *allocator);

#endif /* __ARENA_ALLOCATOR_H__ */