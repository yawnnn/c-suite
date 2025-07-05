/**
 * @file arena.h
 */
#ifndef __ARENA_H__
#define __ARENA_H__

#include <stdint.h>

typedef struct Block Block;

/**
 * @brief arena allocator
 * 
 * should be a bit faster than malloc
 * allows to free memory all at once without keeping track of it
 * or reuse it with arena_reset
 */
typedef struct Arena {
   Block *start; /**< first block (used for free/reset) */
   Block *head; /**< first usable blocks */
} Arena;

/**
 * @brief initialize the arena (equivalent to memset)
 *
 * @param[out] arena allocator
 */
void arena_init(Arena *arena);

/**
 * @brief allocate chunk of memory from the arena
 *
 * @param[in,out] arena allocator
 * @param[in] size size of the chunk in bytes
 * 
 * @return chunk requested or NULL
 */
void *arena_alloc(Arena *arena, size_t size);

/**
 * @brief resize memory previously allocated
 * 
 * WARNING: this is seldom useful since it doesn't free the memory of @p old_block
 *
 * @param[in,out] arena allocator
 * @param[in] new_size new size of the chunk
 * @param[in] old_ptr the previously allocated chunk
 * @param[in] old_size previous chunk size (needed on grow)
 * 
 * @return chunk requested or NULL
 */
void *arena_realloc(Arena *arena, size_t new_size, void *old_ptr, size_t old_size);

/**
 * @brief free all memory in the arena
 *
 * @param[in,out] arena allocator
 */
void arena_deinit(Arena *arena);

/**
 * @brief reset the arena
 * 
 * the memory allocated untill now is to be considered invalid
 * but the arena doesn't free it so it can be reused
 *
 * @param[in,out] arena allocator
 */
void arena_reset(Arena *arena);

#endif /* __ARENA_H__ */