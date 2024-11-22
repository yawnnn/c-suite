#ifndef __ARENA_ALLOCATOR_H__
#define __ARENA_ALLOCATOR_H__

typedef struct Arena {
   unsigned char *start;
   size_t         size;
   size_t         used;
   size_t         nblocks;
} Arena;

typedef struct ArenaList {
   Arena *ptr;
   size_t len;
   size_t first_usable;
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