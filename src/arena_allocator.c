#include "arena_allocator.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#define TRY_REUSE_BLOCK          0

// good enough for everything
#define ALIGNMENT                8

// align `x` to multiple of `alignment`
#define __ALIGN_UP(x, alignment) (((x) + ((alignment) - 1)) & ~((alignment) - 1))

#define ALIGN_PTR(p, alignment)  ((void *)__ALIGN_UP((uintptr_t)p, (uintptr_t)alignment))
#define ALIGN_NUM(n, alignment)  __ALIGN_UP((size_t)n, (size_t)alignment)

#define MAX(a, b)                ((a) > (b) ? (a) : (b))

static inline void   arena_alloc(Arena *arena, size_t size);
static inline void   arena_free(Arena *arena);
static inline bool   arena_has_room(Arena *arena, size_t size);
static inline bool   arena_owns_block(Arena *arena, void *block);
static Arena        *arena_alloc_add(ArenaAlloc *alloc);
static inline Arena *arena_alloc_find_usable(ArenaAlloc *list, size_t size);

#if TRY_REUSE_BLOCK
static inline Arena *arena_alloc_find_owner(ArenaAlloc *list, void *block);
#endif

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

static Arena *arena_alloc_add(ArenaAlloc *alloc) {
   alloc->len++;
   alloc->ptr = (Arena *)realloc(alloc->ptr, alloc->len * sizeof(Arena));
   if (!alloc->ptr)
      return NULL;

   memset(&alloc->ptr[alloc->len - 1], 0, sizeof(Arena));

   return &alloc->ptr[alloc->len - 1];
}

static inline Arena *arena_alloc_find_usable(ArenaAlloc *alloc, size_t size) {
   Arena *curr;

   // the emptiest ones are always gonna be there
   for (size_t i = alloc->len; i > 0; i--) {
      curr = &alloc->ptr[i - 1];
      if (arena_has_room(curr, size))
         return curr;
   }

   curr = &alloc->ptr[alloc->len - 1];
   if (!curr->start)
      return curr;

   return NULL;
}

#if TRY_REUSE_BLOCK
static inline Arena *arena_alloc_find_owner(ArenaAlloc *alloc, void *block) {
   Arena *curr;

   for (size_t i = 0; i < alloc->len; i++) {
      curr = &alloc->ptr[i];
      if (arena_owns_block(curr, block))
         return curr;
   }

   return NULL;
}
#endif

void arena_alloc_init(ArenaAlloc *alloc, size_t min_size) {
   memset(alloc, 0, sizeof(*alloc));
   alloc->min_size = min_size;
   arena_alloc_add(alloc);
}

void *arena_alloc_alloc(ArenaAlloc *alloc, size_t size) {
   Arena         *arena;
   unsigned char *block;

   if (!size)
      return NULL;

   // either here, or lower when increasing head, but then i need to check for surpassing `end`
   size = ALIGN_UP(size, ALIGNMENT);

   arena = arena_alloc_find_usable(alloc, size);
   if (!arena) {
      arena = arena_alloc_add(alloc);
      if (!arena)
         return NULL;
   }

   if (!arena->start) {
      arena_alloc(arena, MAX(alloc->min_size, size));
      if (!arena->start)
         return NULL;
   }

   block = arena->head;
   arena->head += size;

   return (void *)block;
}

void *arena_alloc_realloc(ArenaAlloc *alloc, size_t new_size, void *old_block, size_t old_size) {
   unsigned char *old_block_ = old_block;
   void          *new_block;

   if (!new_size)
      return NULL;

#if TRY_REUSE_BLOCK
   Arena *arena;
   arena = arena_alloc_find_owner(alloc, old_block_);
   if (!arena)
      return NULL;

   if ((size_t)(arena->end - old_block_) <= new_size) {
      new_block = old_block;
      if (old_size < new_size)
         arena->head += ALIGN_UP(new_size - old_size, ALIGNMENT);
      else
         arena->head -= ALIGN_UP(old_size - new_size, ALIGNMENT);
   }
   else {
#endif
      new_block = arena_alloc_alloc(alloc, new_size);
      if (new_block)
         memcpy(new_block, old_block_, old_size);
#if TRY_REUSE_BLOCK
   }
#endif

   return new_block;
}

void arena_alloc_free(ArenaAlloc *alloc, void *block) {}

void arena_alloc_deinit(ArenaAlloc *alloc) {
   while (alloc->len--)
      arena_free(&alloc->ptr[alloc->len]);

   free(alloc->ptr);
   memset(alloc, 0, sizeof(*alloc));
}