#include "arena_allocator.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// pointer size is always good enough
#define DEFAULT_ALIGNMENT                (sizeof(void *))

// align `addr` to multiple of `alignment`
#define ALIGNED_ADDRESS(addr, alignment) (((addr) + (alignment) - 1) & ~((alignment) - 1))

static inline void   arena_alloc(Arena *arena, size_t size);
static inline void   arena_free(Arena *arena);
static inline bool   arena_has_room(Arena *arena, size_t size);
static inline bool   arena_owns_block(Arena *arena, void *block);
static Arena        *arena_list_alloc(ArenaList *list, size_t size);
static void          arena_list_free(ArenaList *list);
static inline Arena *arena_list_find_usable(ArenaList *list, size_t size);
static inline Arena *arena_list_find_corresponding(ArenaList *list, void *block);

static inline void arena_alloc(Arena *arena, size_t size) {
   arena->start = (unsigned char *)malloc(size);
   arena->head = arena->start;
   arena->end = arena->start + size;
}

static inline void arena_free(Arena *arena) {
   free(arena->start);
}

static inline bool arena_has_room(Arena *arena, size_t size) {
   return (size_t)(arena->end - arena->head) >= size;
}

static inline bool arena_owns_block(Arena *arena, void *block) {
   unsigned char *block_ = block;

   return arena->start <= block_ && block_ < arena->head;
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

static inline Arena *arena_list_find_usable(ArenaList *list, size_t size) {
   Arena *curr;

   // start from the bottom, the emptiest ones are always gonna be there
   for (size_t i = list->len; i > 0; i--) {
      curr = &list->ptr[i - 1];
      if (!curr->start || arena_has_room(curr, size))
         return curr;
   }

   return NULL;
}

static inline Arena *arena_list_find_corresponding(ArenaList *list, void *block) {
   Arena *curr;

   for (size_t i = 0; i < list->len; i++) {
      curr = &list->ptr[i];
      if (arena_owns_block(curr, block))
         return curr;
   }

   return NULL;
}

void arena_allocator_init(ArenaAllocator *allocator, size_t default_arena_size) {
   memset(allocator, 0, sizeof(ArenaAllocator));
   allocator->default_arena_size = default_arena_size;
}

void *arena_allocator_alloc(ArenaAllocator *allocator, size_t size) {
   ArenaList     *arena_list = &allocator->arena_list;
   Arena         *arena;
   size_t         arena_size;
   unsigned char *block;

   arena = arena_list_find_usable(arena_list, size);
   if (!arena) {
      arena_size = allocator->default_arena_size < size ? size : allocator->default_arena_size;
      arena = arena_list_alloc(arena_list, arena_size);
   }

   block = arena->head;
   arena->head += ALIGNED_ADDRESS(size, DEFAULT_ALIGNMENT);

   return (void *)block;
}

void *arena_allocator_realloc(ArenaAllocator *allocator, size_t size, void *old_block) {
   ArenaList     *arena_list = &allocator->arena_list;
   Arena         *arena;
   unsigned char *old_block_ = old_block;
   void          *new_block;
   size_t         max_size;

   arena = arena_list_find_corresponding(arena_list, old_block_);
   if (!arena)
      return NULL;

   new_block = arena_allocator_alloc(allocator, size);

   // i don't know the size of the previous allocation, so i just copy as much as it could possibly be
   max_size = arena->end - old_block_;
   memcpy(new_block, old_block_, size > max_size ? max_size : size);

   return new_block;
}

void arena_allocator_free(ArenaAllocator *allocator, void *block) {
   ;
}

void arena_allocator_deinit(ArenaAllocator *allocator) {
   arena_list_free(&allocator->arena_list);
   memset(allocator, 0, sizeof(ArenaAllocator));
}