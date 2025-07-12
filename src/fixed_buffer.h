/**
 * @file fixed_buffer.h
 */
#ifndef __FIXED_BUFFER_H__
#define __FIXED_BUFFER_H__

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief fixed buffer allocator
 * 
 * as fast as it gets, nut has a fixed upper-bound
 * allocates chunks in a user-provided buffer
 * allows to free (reset) memory all at once without keeping track of it
 */
typedef struct FixedBuffer {
   uintptr_t end; /**< end of the buffer */
   uintptr_t head; /**< beginning of free memory */
   void     *beg; /**< beginng of buffer */
} FixedBuffer;

/**
 * @brief initialize allocator
 * 
 * @p buffer can be only changed with a new call to init
 * @p buffer can be on the stack and therefore needs to be freed by the user
 * 
 * note: because of alignment, the effective size will likely be less than @p size
 * 
 * @param[out] fb allocator
 * @param[in] buffer the buffer that will be used
 * @param[in] size size of the bufffer provided
 */
void fixed_buffer_init(FixedBuffer *fb, void *buffer, size_t size);

/**
 * @brief allocate chunk of memory from the buffer
 * 
 * @param[in,out] fb the buffer
 * @param[in] size size of the chunk in bytes
 * 
 * @return chunk requested or NULL
 */
void *fixed_buffer_alloc(FixedBuffer *fb, size_t size);

/**
 * @brief free memory previously allocated
 * 
 * this only does something when @p ptr is the last allocated
 * 
 * @param[in,out] fb allocator
 * @param[in] ptr the previously allocated chunk
 * @param[in] size size of the chunk
 * 
 * @return if the free was successful
 */
bool fixed_buffer_free(FixedBuffer *fb, void *ptr, size_t size);

/**
 * @brief resize memory previously allocated
 * 
 * this is really only useful when @p old_ptr is the last allocated
 * otherwise it equals a normal allocation + memcpy
 *
 * @param[in,out] fb allocator
 * @param[in] new_size new size of the chunk
 * @param[in] old_ptr the previously allocated chunk
 * @param[in] old_size previous chunk size (needed on grow)
 * 
 * @return chunk requested or NULL
 */
void *fixed_buffer_realloc(FixedBuffer *fb, size_t new_size, void *old_ptr, size_t old_size);

/**
 * @brief reset the allocator
 * 
 * the memory allocated untill now is to be considered invalid
 * but now the allocator is fully empty
 * the buffer provided on init is still in use
 *
 * @param[in,out] fb allocator
 */
void fixed_buffer_reset(FixedBuffer *fb);

#endif /* __FIXED_BUFFER_H__ */