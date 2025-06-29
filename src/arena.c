#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"

// TODO: ask user per alloc?
// one-size-fits-all alignment
// beware: types like double and uint64_t can have 8byte alignment even on 32bit
#define ALIGNMENT_T       uint64_t
#define DEFAULT_ALIGNMENT sizeof(ALIGNMENT_T)

typedef struct Block {
   struct Block *next;  // used for free/reset

   uintptr_t   head;  // head of availble memory
   uintptr_t   end;  // end of available memory
   ALIGNMENT_T start[];  // beginning of usable allocation
} Block;

// align `num` to multiple of `alignment`
#define ALIGN_UP(num, alignment) (((num) + ((alignment) - 1)) & ~((alignment) - 1))

// TODO: ask user?
// increasing this means better performance but also more waste of memory
// because blocks are discarded (until reset) when they don't have enough space for the chunk requested
#define MIN_BLOCKS_SIZE   (1024 * 8)

inline static Block *block_new(size_t size)
{
   Block *blk = (Block *)malloc(sizeof(Block) + size);
   if (!blk)
      return NULL;

   blk->next = NULL;
   blk->head = (uintptr_t)blk->start;
   blk->end = (uintptr_t)blk->start + size;

   return blk;
}

inline static void block_free(Block *blk)
{
   do {
      Block *next = blk->next;
      free(blk);
      blk = next;
   } while (blk);
}

inline static void *block_alloc(Block *blk, size_t size)
{
   void *ptr = (void *)blk->head;
   blk->head += ALIGN_UP(size, DEFAULT_ALIGNMENT);

   return ptr;
}

void arena_init(Arena *arena)
{
   arena->start = arena->head = NULL;
}

void arena_deinit(Arena *arena)
{
   block_free(arena->start);
}

void *arena_alloc(Arena *arena, size_t size)
{
   Block *prev = NULL;

   while (arena->head && arena->head->head + size > arena->head->end) {
      prev = arena->head;
      arena->head = arena->head->next;
   }

   if (!arena->head) {
      arena->head = block_new(size > MIN_BLOCKS_SIZE ? size : MIN_BLOCKS_SIZE);
      if (!arena->head)
         return NULL;

      if (prev)
         prev->next = arena->head;
      else
         arena->start = arena->head;
   }

   return block_alloc(arena->head, size);
}

void *arena_realloc(Arena *arena, size_t new_size, void *old_ptr, size_t old_size)
{
   if (new_size <= old_size)
      return old_ptr;

   void *new_ptr = arena_alloc(arena, new_size);
   memcpy(new_ptr, old_ptr, old_size);

   return new_ptr;
}

void arena_reset(Arena *arena)
{
   for (Block *blk = arena->start; blk; blk = blk->next) {
      blk->head = (uintptr_t)blk->start;
   }
   arena->head = arena->start;
}