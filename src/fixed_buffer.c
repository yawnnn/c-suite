#include "fixed_buffer.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief generic alignment
 * 
 * beware: types like double and uint64_t can have 8byte alignment even on 32bit
 */
#define DEFAULT_ALIGNMENT        sizeof(uint64_t)

/**
 * @brief align @p num to multiple of @p alignment
 */
#define ALIGN_UP(num, alignment) (((num) + ((alignment) - 1)) & ~((alignment) - 1))

void fixed_buffer_init(FixedBuffer *fb, void *buffer, size_t size)
{
   fb->start = buffer;
   fb->head = (uintptr_t)buffer;
   fb->end = fb->head + size;
}

void *fixed_buffer_alloc(FixedBuffer *fb, size_t size)
{
   uintptr_t aligned_head = ALIGN_UP(fb->head, DEFAULT_ALIGNMENT);
   if (aligned_head + size > fb->end)
      return NULL;

   void *ptr = (void *)aligned_head;
   fb->head = aligned_head + size;

   return ptr;
}

void *fixed_buffer_realloc(FixedBuffer *fb, size_t new_size, void *old_ptr, size_t old_size)
{
   if (new_size <= old_size) {
      uintptr_t old_head = (uintptr_t)old_ptr;
      if (fb->head == ALIGN_UP(old_head + old_size, DEFAULT_ALIGNMENT))
         fb->head = ALIGN_UP(old_head + new_size, DEFAULT_ALIGNMENT);

      return old_ptr;
   }

   void *new_ptr = fixed_buffer_alloc(fb, new_size);
   memcpy(new_ptr, old_ptr, old_size);

   return new_ptr;
}

void fixed_buffer_reset(FixedBuffer *fb)
{
   fb->head = (uintptr_t)fb->start;
}