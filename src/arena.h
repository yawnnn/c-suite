/**
 * @file arena.h
 */
#ifndef __ARENA_H__
#define __ARENA_H__

#include <stdint.h>

typedef struct Block Block;

/**
 * @brief arena allocator
 */
typedef struct Arena {
   Block *start;  /**< first block (used for free/reset) */
   Block *head;  /**< first usable blocks */
} Arena;

/**
 * @brief initialize the arena (equivalent to memset)
 *
 * @param[out] arena arena
 */
void  arena_init(Arena *arena);

/**
 * @brief malloc memory from the arena
 *
 * @param[in,out] arena arena
 * @param[in] size size of the memory in bytes
 * @return memory requested or NULL
 */
void *arena_alloc(Arena *arena, size_t size);

/**
 * @brief realloc memory previously allocated from the arena
 * 
 * WARNING: this is seldom useful since it doesn't free the memory in @p old_block (untill a call to reset)
 *
 * @param[in,out] arena arena
 * @param[in] new_size new size of the memory
 * @param[in] old_ptr the previously allocated memory
 * @param[in] old_size previous memory size (needed on grow)
 * @return memory requested or NULL
 */
void *arena_realloc(Arena *arena, size_t new_size, void *old_ptr, size_t old_size);

/**
 * @brief free all memory in the arena
 *
 * @param[in,out] arena arena
 */
void  arena_deinit(Arena *arena);

/**
 * @brief reset the arena
 * 
 * the memory allocated untill now is to be considered invalid
 * but the arena doesn't free it so it can be reused
 *
 * @param[in,out] arena arena
 */
void  arena_reset(Arena *arena);

#endif /* __ARENA_H__ */