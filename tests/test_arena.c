#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "arena.h"  // Make sure this has prototypes

void test_arena_init_deinit() {
   Arena arena;
   arena_init(&arena, 1024);
   //assert(arena.min_block_size == 1024);
   //assert(arena.start->end - (uintptr_t)arena.start->start >= 1024);
   arena_deinit(&arena);
   printf("test_arena_init_deinit passed.\n");
}

void test_arena_alloc_basic() {
   Arena arena;
   arena_init(&arena, 1024);
   void *p = arena_alloc(&arena, 128);
   assert(p != NULL);
   assert(arena.head != NULL);
   memset(p, 1, 128);
   arena_deinit(&arena);
   printf("test_arena_alloc_basic passed.\n");
}

void test_arena_alloc_multiple_blocks() {
   Arena arena;
   arena_init(&arena, 64);
   void *a = arena_alloc(&arena, 60);
   void *b = arena_alloc(&arena, 60);  // should trigger new block
   assert(a != NULL && b != NULL);
   assert(a != b);
   memset(a, 1, 60);
   memset(b, 1, 60);
   arena_deinit(&arena);
   printf("test_arena_alloc_multiple_blocks passed.\n");
}

void test_arena_alloc_zero() {
   Arena arena;
   arena_init(&arena, 1024);
   arena_alloc(&arena, 0);   // returns non-NULL
   arena_deinit(&arena);
   printf("test_arena_alloc_zero passed.\n");
}

void test_arena_realloc_grow() {
   Arena arena;
   arena_init(&arena, 1024);
   void *p1 = arena_alloc(&arena, 100);
   void *p1_2 = arena_alloc(&arena, 100);
   assert(p1);
   memset(p1, 42, 100);
   memset(p1_2, 42, 100);
   void *p2 = arena_realloc(&arena, 200, p1, 100);
   assert(p2 != NULL);
   assert(memcmp(p2, p1_2, 100) == 0);  // data copied
   arena_deinit(&arena);
   printf("test_arena_realloc_grow passed.\n");
}

void test_arena_realloc_shrink() {
   Arena arena;
   arena_init(&arena, 1024);
   void *p1 = arena_alloc(&arena, 200);
   void *p1_2 = arena_alloc(&arena, 200);
   assert(p1);
   memset(p1, 42, 200);
   memset(p1_2, 42, 200);
   void *p2 = arena_realloc(&arena, 100, p1, 200);
   assert(p2 != NULL);
   assert(memcmp(p2, p1_2, 100) == 0);
   arena_deinit(&arena);
   printf("test_arena_realloc_shrink passed.\n");
}

int main() {
   printf("Arena:\n");

   test_arena_init_deinit();
   test_arena_alloc_basic();
   test_arena_alloc_multiple_blocks();
   test_arena_alloc_zero();
   test_arena_realloc_grow();
   test_arena_realloc_shrink();

   printf("\nAll tests passed.\n");
   return 0;
}