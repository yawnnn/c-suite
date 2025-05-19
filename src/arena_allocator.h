#ifndef __ARENA_ALLOCATOR_H__
#define __ARENA_ALLOCATOR_H__

#include <stdint.h>

typedef struct Arena {
   unsigned char *start;
   unsigned char *head;
   unsigned char *end;
} Arena;

typedef struct ArenaAlloc {
   Arena *ptr;
   size_t len;
   size_t min_size;
} ArenaAlloc;

void  arena_alloc_init(ArenaAlloc *alloc, size_t min_size);
void *arena_alloc_alloc(ArenaAlloc *alloc, size_t size);
void *arena_alloc_realloc(ArenaAlloc *alloc, size_t new_size, void *old_block, size_t old_size);
void  arena_alloc_free(ArenaAlloc *alloc, void *block);
void  arena_alloc_deinit(ArenaAlloc *alloc);

#endif /* __ARENA_ALLOCATOR_H__ */