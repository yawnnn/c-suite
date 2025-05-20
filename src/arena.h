#ifndef __ARENA_H__
#define __ARENA_H__

#include <stdint.h>

typedef struct Block {
   void *start;  // beginning of allocation
   size_t used;  // used space
   size_t size;  // total space

   struct Block *next;  // free list
} Block;

typedef struct Arena {
   Block *first;  // beginning of free list
   Block *curr;  // last arena used
   size_t min_block_size;  // minimum block size

#if __DEBUG
   size_t tot_blocks;
   size_t tot_req;
   size_t tot_used;
#endif
} Arena;

void  arena_init(Arena *alloc, size_t min_block_size);
void *arena_alloc(Arena *alloc, size_t size);
void *arena_realloc(Arena *alloc, size_t new_size, void *old_block, size_t old_size);
void  arena_deinit(Arena *alloc);

#endif /* __ARENA_H__ */