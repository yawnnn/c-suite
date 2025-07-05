#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "arena.h"

void test_arena_init_deinit() {
   Arena arena;
   arena_init(&arena);
   arena_deinit(&arena);
   printf("%s passed.\n", __func__);
}

void test_arena_alloc_basic() {
   Arena arena;
   arena_init(&arena);
   void *p = arena_alloc(&arena, 128);
   assert(p != NULL);
   assert(arena.head != NULL);
   memset(p, 1, 128);
   arena_deinit(&arena);
   printf("%s passed.\n", __func__);
}

void test_arena_alloc_multiple_blocks() {
   Arena arena;
   arena_init(&arena);
   void *a = arena_alloc(&arena, 60);
   void *b = arena_alloc(&arena, 60);
   assert(a != NULL && b != NULL);
   assert(a != b);
   memset(a, 1, 60);
   memset(b, 1, 60);
   arena_deinit(&arena);
   printf("%s passed.\n", __func__);
}

void test_arena_alloc_zero() {
   Arena arena;
   arena_init(&arena);
   arena_alloc(&arena, 0);
   arena_deinit(&arena);
   printf("%s passed.\n", __func__);
}

void test_arena_realloc_grow() {
   Arena arena;
   arena_init(&arena);
   void *p1 = arena_alloc(&arena, 100);
   void *p1_2 = arena_alloc(&arena, 100);
   assert(p1);
   memset(p1, 42, 100);
   memset(p1_2, 42, 100);
   void *p2 = arena_realloc(&arena, 200, p1, 100);
   assert(p2 != NULL);
   assert(memcmp(p2, p1_2, 100) == 0);
   arena_deinit(&arena);
   printf("%s passed.\n", __func__);
}

void test_arena_realloc_shrink() {
   Arena arena;
   arena_init(&arena);
   void *p1 = arena_alloc(&arena, 200);
   void *p1_2 = arena_alloc(&arena, 200);
   assert(p1);
   memset(p1, 42, 200);
   memset(p1_2, 42, 200);
   void *p2 = arena_realloc(&arena, 100, p1, 200);
   assert(p2 != NULL);
   assert(memcmp(p2, p1_2, 100) == 0);
   arena_deinit(&arena);
   printf("%s passed.\n", __func__);
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