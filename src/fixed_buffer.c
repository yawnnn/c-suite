#include "fixed_buffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(__STDC__) && __STDC_VERSION__ >= 201112L
   #include "stdalign.h"
   #define DEFAULT_ALIGNMENT alignof(max_align_t)
#elif defined(_MSC_VER)
    #define DEFAULT_ALIGNMENT __alignof(max_align_t)
#else
   /**
    * @brief generic alignment
    * 
    * note: types like double and uint64_t can have 8byte alignment even on 32bit
    */
   #define DEFAULT_ALIGNMENT 8
#endif

/**
 * @brief align @p num to multiple of @p alignment
 */
#define ALIGN_UP(num, alignment) (((num) + ((alignment) - 1)) & ~((alignment) - 1))

void fixed_buffer_init(FixedBuffer *fb, void *buffer, size_t size)
{
   fb->beg = buffer;
   fb->head = (uintptr_t)buffer;
   fb->end = fb->head + size;
}

void *fixed_buffer_alloc(FixedBuffer *fb, size_t size)
{
   if (fb->head + size > fb->end)
      return NULL;

   void *ptr = (void *)fb->head;
   fb->head += ALIGN_UP(size, DEFAULT_ALIGNMENT);

   return ptr;
}

bool fixed_buffer_free(FixedBuffer *fb, void *ptr, size_t size)
{
   void *last_ptr = (void *)(fb->head - ALIGN_UP(size, DEFAULT_ALIGNMENT));
   if (ptr != last_ptr)
      return false;

   fb->head = (uintptr_t)ptr;

   return true;
}

void *fixed_buffer_realloc(FixedBuffer *fb, size_t new_size, void *old_ptr, size_t old_size)
{
   bool is_same_ptr = fixed_buffer_free(fb, old_ptr, old_size);
   if (new_size <= old_size && !is_same_ptr)
      return old_ptr;

   void *new_ptr = fixed_buffer_alloc(fb, new_size);
   if (new_ptr && !is_same_ptr)
      memcpy(new_ptr, old_ptr, old_size);

   return new_ptr;
}

void fixed_buffer_reset(FixedBuffer *fb)
{
   fb->head = (uintptr_t)fb->beg;
}