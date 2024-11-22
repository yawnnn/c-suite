#include "arena_allocator.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

inline static void arena_alloc(Arena *arena, size_t size);
inline static void arena_free(Arena *arena);
static Arena *arena_list_alloc(ArenaList *list, size_t size);
static void arena_list_free(ArenaList *list);
static Arena *arena_list_find_empty(ArenaList *list, size_t size);
static Arena *arena_list_find_corresponding(ArenaList *list, void *block);

inline static void arena_alloc(Arena *arena, size_t size) {
   arena->start = (unsigned char *)malloc(size);
   arena->size = size;
   arena->used = arena->nblocks = 0;
}

inline static void arena_free(Arena *arena) {
   free(arena->start);
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

static Arena *arena_list_find_empty(ArenaList *list, size_t size) {
   Arena *arena = NULL;

   for (size_t i = 0; i < list->len; i++) {
      // `Arena::used` could be bigger than `Arena::size` because of alignment
      if (!list->ptr[i].start || list->ptr[i].used + size < list->ptr[i].size) {
         arena = &list->ptr[i];
         break;
      }
   }

   return arena;
}

static Arena *arena_list_find_corresponding(ArenaList *list, void *block) {
   Arena *arena = NULL;
   unsigned char *block_ = block;

   for (size_t i = 0; i < list->len; i++) {
      if (list->ptr[i].start >= block_ && block_ - list->ptr[i].start < list->ptr[i].used) {
         arena = &list->ptr[i];
         break;
      }
   }

   return arena;
}

void arena_allocator_init(ArenaAllocator *allocator, size_t default_arena_size) {
   memset(allocator, 0, sizeof(ArenaAllocator));
   allocator->default_arena_size = default_arena_size;
}

void *arena_allocator_alloc(ArenaAllocator *allocator, size_t size) {
   ArenaList *arena_list = &allocator->arena_list;
   Arena *arena;
   size_t required_size, block_size;
   void *block;

   // tag + block
   required_size = 1 + size;

   arena = arena_list_find_empty(arena_list, required_size);
   if (!arena) {
      block_size = allocator->default_arena_size < required_size ? required_size
                                                                 : allocator->default_arena_size;
      arena = arena_list_alloc(arena_list, block_size);
   }

   // tag, to detect double free
   arena->start[arena->used] = (unsigned char)0;
   arena->used++;

   block = &arena->start[arena->used];
   arena->used += size;
   arena->nblocks++;

   // aligning to a multiple of sizeof(void *), which should be good enough for everything
   arena->used = (arena->used + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

   return block;
}

void arena_allocator_realloc(ArenaAllocator *allocator, size_t size, void *old_block) {
   ArenaList *arena_list = &allocator->arena_list;
   Arena *arena;
   void *new_block;
   size_t max_old_block_size;

   arena = arena_list_find_corresponding(arena_list, old_block);
   if (!arena)
      return NULL;

   new_block = arena_allocator_alloc(allocator, size);

   // i don't know the exact size of the previous old_block, so i just copy as much as i have
   max_old_block_size = (size_t)((arena->start + arena->size) - old_block);
   max_old_block_size = max_old_block_size > size ? size : max_old_block_size;
   memcpy(new_block, old_block, max_old_block_size);

   arena_allocator_free(allocator, old_block);

   return new_block;
}

void arena_allocator_free(ArenaAllocator *allocator, void *block) {
   ArenaList *arena_list = &allocator->arena_list;
   Arena *arena;
   size_t i;
   unsigned char *tag;

   // check if i allocated it
   arena = arena_list_find_corresponding(arena_list, block);
   if (!arena)
      return;

   // check double free
   tag = (unsigned char *)block - 1;
   if (*tag)
      return;

   *tag = 1;
   arena->nblocks--;

   if (!arena->nblocks) {
      arena_free(arena);
      memset(arena, 0, sizeof(Arena));
   }
}

void arena_allocator_reset(ArenaAllocator *allocator) {
   arena_list_free(allocator);
   memset(allocator, 0, sizeof(ArenaAllocator));
}