#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "arena.h"

void test_arena_init_deinit() {
   Arena arena;
   arena_init(&arena);
   arena_deinit(&arena);
   printf("%s passed.\n", __func__);
}

void test_arena_alloc_basic()
{
   Arena arena;
   arena_init(&arena);

   void *ptr = arena_alloc(&arena, 64);
   assert(ptr != NULL);

   memset(ptr, 0xAB, 64);
   for (int i = 0; i < 64; ++i) {
      assert(((unsigned char *)ptr)[i] == 0xAB);
   }

   arena_deinit(&arena);
   
   printf("%s passed\n", __func__);
}

void test_arena_alloc_zero_size()
{
   Arena arena;
   arena_init(&arena);

   void *ptr1 = arena_alloc(&arena, 0);
   void *ptr2 = arena_alloc(&arena, 0);
   assert(ptr1 == ptr2);

   arena_deinit(&arena);
   
   printf("%s passed\n", __func__);
}

void test_arena_free_last_alloc()
{
   Arena arena;
   arena_init(&arena);

   void *ptr = arena_alloc(&arena, 128);
   assert(ptr != NULL);

   bool success = arena_free(&arena, ptr, 128);
   assert(success);

   // Allocation should reuse same memory if possible
   void *ptr2 = arena_alloc(&arena, 128);
   assert(ptr2 == ptr);

   arena_deinit(&arena);
   
   printf("%s passed\n", __func__);
}

void test_arena_free_non_last_alloc()
{
   Arena arena;
   arena_init(&arena);

   void *ptr1 = arena_alloc(&arena, 64);
   void *ptr2 = arena_alloc(&arena, 64);

   assert(ptr1 != NULL && ptr2 != NULL);

   bool success = arena_free(&arena, ptr1, 64);
   assert(!success);

   arena_deinit(&arena);
   
   printf("%s passed\n", __func__);
}

void test_arena_realloc_grow_last()
{
   Arena arena;
   arena_init(&arena);

   char *ptr = arena_alloc(&arena, 16);
   assert(ptr != NULL);

   strcpy(ptr, "hello");

   char *new_ptr = arena_realloc(&arena, 32, ptr, 16);
   assert(new_ptr != NULL);
   assert(strcmp(new_ptr, "hello") == 0);

   arena_deinit(&arena);
   
   printf("%s passed\n", __func__);
}

void test_arena_realloc_non_last()
{
   Arena arena;
   arena_init(&arena);

   char *a = arena_alloc(&arena, 32);
   arena_alloc(&arena, 32);
   strcpy(a, "world");

   char *new_ptr = arena_realloc(&arena, 64, a, 32);
   assert(new_ptr != NULL);
   assert(strcmp(new_ptr, "world") == 0);
   assert(new_ptr != a);  // Should not be same pointer since `a` wasn't last

   arena_deinit(&arena);
   
   printf("%s passed\n", __func__);
}

void test_arena_reset_and_reuse()
{
   Arena arena;
   arena_init(&arena);

   void *ptr1 = arena_alloc(&arena, 128);
   assert(ptr1 != NULL);

   arena_reset(&arena);

   void *ptr2 = arena_alloc(&arena, 128);
   assert(ptr2 != NULL);

   // Might or might not reuse same block, but must be usable
   memset(ptr2, 0xCC, 128);

   arena_deinit(&arena);
   
   printf("%s passed\n", __func__);
}

void test_arena_deinit_no_leaks()
{
   Arena arena;
   arena_init(&arena);

   for (int i = 0; i < 100; ++i) {
      void *p = arena_alloc(&arena, 1024);
      assert(p != NULL);
      memset(p, i, 1024);
   }

   arena_deinit(&arena);
   
   printf("%s passed\n", __func__);
}

int main()
{
   test_arena_alloc_basic();
   test_arena_alloc_zero_size();
   test_arena_free_last_alloc();
   test_arena_free_non_last_alloc();
   test_arena_realloc_grow_last();
   test_arena_realloc_non_last();
   test_arena_reset_and_reuse();
   test_arena_deinit_no_leaks();
   
   printf("%s suite passed!\n", __FILE__);
   return 0;
}
