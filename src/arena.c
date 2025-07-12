#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"

/**
 * @brief generic alignment
 * 
 * note: types like double and uint64_t can have 8byte alignment even on 32bit
 */
#define DEFAULT_ALIGNMENT sizeof(uint64_t)

/**
 * @brief tracks block of memory allocated
 */
typedef struct Block {
   struct Block *next; /**< intrusive free list */

   uintptr_t end; /**< end of allocation */
   uintptr_t head; /**< beginning of free memory */
   char     *beg[]; /**< beginning of allocation */
} Block;

/**
 * @brief align @p num to multiple of @p alignment
 */
#define ALIGN_UP(num, alignment) (((num) + ((alignment) - 1)) & ~((alignment) - 1))

/**
 * @brief minimum block asked to malloc at a time
 * 
 * increasing this means better performance but also more (potential) waste of memory
 * because blocks are discarded (until reset) when they don't have enough space for the chunk requested
 */
#define MIN_BLOCKS_SIZE          (1024 * 8)

/**
 * @brief alloc and init a block
 * 
 * @param[in] size block size
 * 
 * @return allocated block
 */
inline static Block *block_new(size_t size)
{
   Block *blk = (Block *)malloc(sizeof(Block) + size);
   if (!blk)
      return NULL;

   blk->next = NULL;
   blk->head = (uintptr_t)blk->beg;
   blk->end = (uintptr_t)blk->beg + size;

   return blk;
}

/**
 * @brief free block
 * 
 * @param[in,out] blk block to be freed
 */
inline static void block_free(Block *blk)
{
   do {
      Block *next = blk->next;
      free(blk);
      blk = next;
   } while (blk);
}

/**
 * @brief allocate a chunk from inside a block
 * 
 * @param[in,out] blk block
 * @param[in] size size of the chunk
 * 
 * @return allocated chunk
 */
inline static void *block_alloc(Block *blk, size_t size)
{
   void *ptr = (void *)blk->head;
   blk->head += ALIGN_UP(size, DEFAULT_ALIGNMENT);

   return ptr;
}

/**
 * @brief check is block has @p size free bytes
 * 
 * @param[in] blk block
 * @param[in] size number of bytes
 * 
 * @return boolean
 */
inline static bool block_has_room(Block *blk, size_t size)
{
   return blk->head + size <= blk->end;
}

void arena_deinit(Arena *arena)
{
   if (arena->free_list)
      block_free(arena->free_list);
}

void *arena_alloc(Arena *arena, size_t size)
{
   Block *prev = NULL;

   while (arena->curr && !block_has_room(arena->curr, size)) {
      prev = arena->curr;
      arena->curr = arena->curr->next;
   }

   if (!arena->curr) {
      arena->curr = block_new(size > MIN_BLOCKS_SIZE ? size : MIN_BLOCKS_SIZE);
      if (!arena->curr)
         return NULL;

      if (prev)
         prev->next = arena->curr;
      else
         arena->free_list = arena->curr;
   }

   return block_alloc(arena->curr, size);
}

bool arena_free(Arena *arena, void *ptr, size_t size)
{
   if (!arena->curr)
      return false;

   void *last_ptr = (void *)(arena->curr->head - ALIGN_UP(size, DEFAULT_ALIGNMENT));
   if (ptr != last_ptr)
      return false;

   arena->curr->head = (uintptr_t)ptr;

   return true;
}

void *arena_realloc(Arena *arena, size_t new_size, void *old_ptr, size_t old_size)
{
   bool is_same_ptr = arena_free(arena, old_ptr, old_size);
   if (new_size <= old_size && !is_same_ptr)
      return old_ptr;

   void *new_ptr = arena_alloc(arena, new_size);
   if (new_ptr && !is_same_ptr)
      memcpy(new_ptr, old_ptr, old_size);

   return new_ptr;
}

void arena_reset(Arena *arena)
{
   for (Block *blk = arena->free_list; blk; blk = blk->next) {
      blk->head = (uintptr_t)blk->beg;
   }
   arena->curr = arena->free_list;
}