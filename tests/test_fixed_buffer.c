#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "fixed_buffer.h"

#define BUFFER_SIZE 1024

void test_fixed_buffer_alloc_basic()
{
   FixedBuffer fb;
   char        buffer[BUFFER_SIZE];

   fixed_buffer_init(&fb, buffer, BUFFER_SIZE);

   void *ptr = fixed_buffer_alloc(&fb, 128);
   assert(ptr != NULL);

   memset(ptr, 0xAA, 128);
   for (int i = 0; i < 128; ++i)
      assert(((unsigned char *)ptr)[i] == 0xAA);

   printf("%s passed\n", __func__);
}

void test_fixed_buffer_alloc_zero_size()
{
   FixedBuffer fb;
   char        buffer[BUFFER_SIZE];

   fixed_buffer_init(&fb, buffer, BUFFER_SIZE);

   void *ptr1 = fixed_buffer_alloc(&fb, 0);
   void *ptr2 = fixed_buffer_alloc(&fb, 0);
   assert(ptr1 == ptr2);

   printf("%s passed\n", __func__);
}

void test_fixed_buffer_alloc_overflow()
{
   FixedBuffer fb;
   char        buffer[BUFFER_SIZE];

   fixed_buffer_init(&fb, buffer, BUFFER_SIZE);

   void *ptr = fixed_buffer_alloc(&fb, BUFFER_SIZE * 2);
   assert(ptr == NULL);

   printf("%s passed\n", __func__);
}

void test_fixed_buffer_free_last_alloc()
{
   FixedBuffer fb;
   char        buffer[BUFFER_SIZE];

   fixed_buffer_init(&fb, buffer, BUFFER_SIZE);

   void *ptr = fixed_buffer_alloc(&fb, 256);
   assert(ptr != NULL);

   bool freed = fixed_buffer_free(&fb, ptr, 256);
   assert(freed);

   void *ptr2 = fixed_buffer_alloc(&fb, 256);
   assert(ptr2 == ptr);

   printf("%s passed\n", __func__);
}

void test_fixed_buffer_free_non_last_alloc()
{
   FixedBuffer fb;
   char        buffer[BUFFER_SIZE];

   fixed_buffer_init(&fb, buffer, BUFFER_SIZE);

   void *a = fixed_buffer_alloc(&fb, 128);
   void *b = fixed_buffer_alloc(&fb, 128);
   assert(a && b);

   bool freed = fixed_buffer_free(&fb, a, 128);
   assert(!freed);  // Not last

   printf("%s passed\n", __func__);
}

void test_fixed_buffer_realloc_last()
{
   FixedBuffer fb;
   char        buffer[BUFFER_SIZE];

   fixed_buffer_init(&fb, buffer, BUFFER_SIZE);

   char *ptr = fixed_buffer_alloc(&fb, 64);
   assert(ptr);

   strcpy(ptr, "hello");

   char *new_ptr = fixed_buffer_realloc(&fb, 128, ptr, 64);
   assert(new_ptr);
   assert(strcmp(new_ptr, "hello") == 0);

   printf("%s passed\n", __func__);
}

void test_fixed_buffer_realloc_non_last()
{
   FixedBuffer fb;
   char        buffer[BUFFER_SIZE];

   fixed_buffer_init(&fb, buffer, BUFFER_SIZE);

   char *a = fixed_buffer_alloc(&fb, 64);
   char *b = fixed_buffer_alloc(&fb, 64);
   assert(a && b);

   strcpy(a, "foo");

   char *new_ptr = fixed_buffer_realloc(&fb, 128, a, 64);
   assert(new_ptr != NULL);
   assert(strcmp(new_ptr, "foo") == 0);
   assert(new_ptr != a);

   printf("%s passed\n", __func__);
}

void test_fixed_buffer_reset()
{
   FixedBuffer fb;
   char        buffer[BUFFER_SIZE];

   fixed_buffer_init(&fb, buffer, BUFFER_SIZE);

   void *ptr1 = fixed_buffer_alloc(&fb, 512);
   assert(ptr1);

   fixed_buffer_reset(&fb);

   void *ptr2 = fixed_buffer_alloc(&fb, 512);
   assert(ptr2 != NULL);

   // Might reuse the memory depending on alignment
   memset(ptr2, 0xCC, 512);

   printf("%s passed\n", __func__);
}

void test_fixed_buffer_exhaustion()
{
   FixedBuffer fb;
   char        buffer[BUFFER_SIZE];

   fixed_buffer_init(&fb, buffer, BUFFER_SIZE);

   size_t alloc_size = 128;
   int    count = 0;

   while (fixed_buffer_alloc(&fb, alloc_size)) {
      count++;
   }

   assert(count <= BUFFER_SIZE / alloc_size);
   printf("%s passed\n", __func__);
}

int main()
{
   test_fixed_buffer_alloc_basic();
   test_fixed_buffer_alloc_zero_size();
   test_fixed_buffer_alloc_overflow();
   test_fixed_buffer_free_last_alloc();
   test_fixed_buffer_free_non_last_alloc();
   test_fixed_buffer_realloc_last();
   test_fixed_buffer_realloc_non_last();
   test_fixed_buffer_reset();
   test_fixed_buffer_exhaustion();

   printf("All FixedBuffer tests passed!\n");
   return 0;
}
