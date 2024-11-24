#include "arena_allocator.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// pointer size is always good enough
#define DEFAULT_ALIGNMENT                     (sizeof(void *))

// align `addr` to multiple of `alignment`
#define ALIGNED_ADDRESS(addr, alignment)      (((addr) + (alignment) - 1) & ~((alignment) - 1))

// the actual tag data + the eventual padding to align with `alignment`
#define MAX_TAG_SIZE(min_tag_size, alignment) ((min_tag_size) + (alignment) - 1)

static inline void           arena_alloc(Arena *arena, size_t size);
static inline void           arena_free(Arena *arena);
static inline bool           arena_has_room(Arena *arena, size_t size);
static inline bool           arena_owns_block(Arena *arena, void *block);
static inline unsigned char *arena_head(Arena *arena);
static inline unsigned char *arena_end(Arena *arena);
static Arena                *arena_list_alloc(ArenaList *list, size_t size);
static void                  arena_list_free(ArenaList *list);
static Arena                *arena_list_find_usable(ArenaList *list, size_t size);
static Arena                *arena_list_find_corresponding(ArenaList *list, void *block);
static void                  arena_list_increased_usage(ArenaList *list, Arena *arena);
static inline void           arena_list_decreased_usage(ArenaList *list, Arena *arena);

static inline void arena_alloc(Arena *arena, size_t size) {
   arena->start = (unsigned char *)malloc(size);
   arena->size = size;
   arena->used = arena->nblocks = 0;
}

static inline void arena_free(Arena *arena) {
   free(arena->start);
}

static inline bool arena_has_room(Arena *arena, size_t size) {
   return arena->size - arena->used >= size;
}

static inline bool arena_owns_block(Arena *arena, void *block) {
   return arena->start <= block && arena->start + arena->used > block;
}

static inline unsigned char *arena_head(Arena *arena) {
   return arena->start + arena->used;
}

static inline unsigned char *arena_end(Arena *arena) {
   return arena->start + arena->size;
}

static Arena *arena_list_alloc(ArenaList *list, size_t size) {
   Arena *arena;

   list->len++;
   list->ptr = (Arena *)realloc(list->ptr, list->len * sizeof(Arena));

   arena = &list->ptr[list->len - 1];
   arena_alloc(arena, size);

   return arena;
}

static void arena_list_free(ArenaList *list) {
   while (list->len--)
      arena_free(&list->ptr[list->len]);

   if (list->ptr)
      free(list->ptr);
}

static Arena *arena_list_find_usable(ArenaList *list, size_t size) {
   Arena *curr;

   for (size_t i = list->first_usable; i < list->len; i++) {
      curr = &list->ptr[i];
      if (!curr->start || arena_has_room(curr, size))
         return curr;
   }

   return NULL;
}

static Arena *arena_list_find_corresponding(ArenaList *list, void *block) {
   Arena *curr;

   for (size_t i = 0; i < list->len; i++) {
      curr = &list->ptr[i];
      if (arena_owns_block(curr, block))
         return curr;
   }

   return NULL;
}

static void arena_list_increased_usage(ArenaList *list, Arena *arena) {
   // only if it previosly was the first usable
   if (arena == &list->ptr[list->first_usable]) {
      const size_t min_occupancy = 1 + MAX_TAG_SIZE(sizeof(size_t), DEFAULT_ALIGNMENT);
      Arena       *curr;
      size_t       i;

      for (i = list->first_usable; i < list->len; i++) {
         curr = &list->ptr[i];
         if (!curr->start || arena_has_room(curr, min_occupancy))
            break;
      }

      list->first_usable = i;
   }
}

static inline void arena_list_decreased_usage(ArenaList *list, Arena *arena) {
   const size_t min_occupancy = 1 + MAX_TAG_SIZE(sizeof(size_t), DEFAULT_ALIGNMENT);

   if (arena < &list->ptr[list->first_usable]
       && (!arena->start || arena_has_room(arena, min_occupancy)))
   {
      list->first_usable = (size_t)(list->ptr - arena);
   }
}

void arena_allocator_init(ArenaAllocator *allocator, size_t default_arena_size) {
   memset(allocator, 0, sizeof(ArenaAllocator));
   allocator->default_arena_size = default_arena_size;
}

void *arena_allocator_alloc(ArenaAllocator *allocator, size_t size) {
   ArenaList     *arena_list = &allocator->arena_list;
   Arena         *arena;
   size_t         max_required_size, arena_size;
   uintptr_t      head_addr;
   unsigned char *block;

   if (!size)
      return NULL;

   max_required_size = MAX_TAG_SIZE(sizeof(size_t), DEFAULT_ALIGNMENT) + size;

   arena = arena_list_find_usable(arena_list, max_required_size);
   if (!arena || !arena->start) {
      arena_size = allocator->default_arena_size < max_required_size
         ? max_required_size
         : allocator->default_arena_size;

      if (!arena)
         arena = arena_list_alloc(arena_list, arena_size);
      else
         arena_alloc(arena, arena_size);
   }

   head_addr = (uintptr_t)(arena->start + arena->used);

   // tag data
   head_addr += sizeof(size_t);

   // align the block for performance
   head_addr = ALIGNED_ADDRESS(head_addr, DEFAULT_ALIGNMENT);

   block = (unsigned char *)head_addr;

   // writing down block size. i need this for realloc and safeguard against double free
   *(size_t *)(block - sizeof(size_t)) = size;
   arena->used = (size_t)(block + size - arena->start);
   arena->nblocks++;

   arena_list_increased_usage(arena_list, arena);

   return (void *)block;
}

void *arena_allocator_realloc(ArenaAllocator *allocator, size_t size, void *old_block) {
   ArenaList     *arena_list = &allocator->arena_list;
   Arena         *arena;
   unsigned char *old_block_ = old_block;
   void          *new_block;
   size_t         old_size, size_growth;

   arena = arena_list_find_corresponding(arena_list, old_block_);
   if (!arena)
      return NULL;

   old_size = *(size_t *)(old_block_ - sizeof(size_t));

   if (size < old_size) {
      // shrink, so i reuse the same block
      arena->used -= old_size - size;
      new_block = old_block;

      arena_list_decreased_usage(arena_list, arena);
   }
   else {
      size_growth = size - old_size;

      if (!size_growth)
         new_block = old_block;
      else if (arena_head(arena) == old_block_ + old_size && arena_has_room(arena, size_growth)) {
         // grow, but i can extend the current
         arena->used += size_growth;
         new_block = old_block;

         arena_list_increased_usage(arena_list, arena);
      }
      else {
         new_block = arena_allocator_alloc(allocator, size);
         memcpy(new_block, old_block, old_size);
         arena_allocator_free(allocator, old_block);

         arena_list_increased_usage(arena_list, arena);
      }
   }

   return new_block;
}

void arena_allocator_free(ArenaAllocator *allocator, void *block) {
   ArenaList     *arena_list = &allocator->arena_list;
   Arena         *arena;
   size_t         i, *psize;
   unsigned char *block_ = block;

   // check if i allocated it
   arena = arena_list_find_corresponding(arena_list, block_);
   if (!arena)
      return;

   psize = (size_t *)(block_ - sizeof(size_t));
   if (!*psize)
      return;

   *psize = 0;
   arena->nblocks--;

   if (!arena->nblocks) {
      arena_free(arena);
      memset(arena, 0, sizeof(Arena));

      arena_list_decreased_usage(arena_list, arena);
   }
}

void arena_allocator_deinit(ArenaAllocator *allocator) {
   arena_list_free(&allocator->arena_list);
   memset(allocator, 0, sizeof(ArenaAllocator));
}