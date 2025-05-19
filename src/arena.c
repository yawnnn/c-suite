#include "arena.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

// good enough for everything
#define DEFAULT_ALIGNMENT        8

// align `num` to multiple of `alignment`
#define ALIGN_UP(num, alignment) (((num) + ((alignment) - 1)) & ~((alignment) - 1))

// increasing this means better performance but also more waste of memory
// es. (WORST CASE) the following will always waste the remaining first block
//     arena_alloc((DEFAULT_BLOCK_SIZE/2)+1);      // first block
//     arena_alloc((DEFAULT_BLOCK_SIZE/2));        // second block
#define DEFAULT_BLOCK_SIZE       8192

static Block *block_new(size_t size) {
   Block *blk;

   blk = (Block *)malloc(sizeof(Block) + size);
   if (!blk)
      return NULL;

   blk->start = (void *)(blk + 1);
   blk->used = 0;
   blk->size = size;
   blk->next = NULL;

   return blk;
}

static void block_free(Block *blk) {
   Block *next;

   do {
      next = blk->next;
      free(blk);
      blk = next;
   } while (blk);
}

static void *block_alloc(Block *blk, size_t size, size_t min_block_size) {
   void *ptr;

   if (blk->used + size > blk->size) {
      Block *new_blk;

      new_blk = block_new(size > min_block_size ? size : min_block_size);
      if (!new_blk)
         return NULL;

      blk->next = new_blk;
      blk->used = size;
      blk = new_blk;
   }

   ptr = (void *)((char *)blk->start + blk->used);
   blk->used += ALIGN_UP(size, DEFAULT_ALIGNMENT);

   return ptr;
}

void arena_init(Arena *arena, size_t min_block_size) {
   arena->min_block_size = min_block_size ? min_block_size : DEFAULT_BLOCK_SIZE;
   arena->first = block_new(arena->min_block_size);

#if __DEBUG
   arena->tot_blocks = 1;
   arena->tot_used = arena->first->size;
   arena->tot_req = 0;
#endif

   arena->curr = arena->first;
}

void arena_deinit(Arena *arena) {
   block_free(arena->first);
}

void *arena_alloc(Arena *arena, size_t size) {
   Block *next;
   void  *ptr;

   ptr = block_alloc(arena->curr, size, arena->min_block_size);
   if (!ptr)
      return NULL;

   // if there's a new block
   next = arena->curr->next;
   if (next) {
      // the new curr is the one with that's less used
      if (arena->curr->used > next->used)
         arena->curr = next;
      else {
         next->next = arena->first;
         arena->first = next;
         arena->curr->next = NULL;
      }

#if __DEBUG
      arena->tot_blocks++;
      arena->tot_used += next->size;
#endif
   }

#if __DEBUG
   arena->tot_req += size;
#endif

   return ptr;
}

void *arena_realloc(Arena *arena, size_t new_size, void *old_ptr, size_t old_size) {
   void *new_ptr;

   if (new_size <= old_size)
      return old_ptr;

   new_ptr = arena_alloc(arena, new_size);
   memcpy(new_ptr, old_ptr, old_size);

   return new_ptr;
}