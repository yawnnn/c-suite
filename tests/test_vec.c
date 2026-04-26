#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vec.h"

#if defined(_MSC_VER)
   #if defined(strdup)
      #undef strdup
   #endif
   #define strdup _strdup
#endif

void test_vec_new_and_empty()
{
   Vec v;
   size_t len;
   vec_new(&v, sizeof(int), NULL);
   assert(vec_slice(&v, &len) == NULL);
   assert(vec_is_empty(&v));
   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_new_with()
{
   Vec v;
   size_t len;
   vec_new_with(&v, sizeof(int), 10, NULL);
   assert(vec_slice(&v, &len) == NULL);  // Still 0-length
   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_from()
{
   int arr[] = {1, 2, 3};
   Vec v;
   size_t len;
   vec_from(&v, sizeof(int), arr, 3, NULL);
   assert(v.len == 3);
   int *data = vec_slice(&v, &len);
   for (int i = 0; i < 3; i++)
      assert(data[i] == arr[i]);
   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_push_and_elem_at()
{
   Vec v;
   vec_new(&v, sizeof(int), NULL);
   int a = 42;
   vec_push(&v, &a);
   assert(v.len == 1);
   int *elem = (int *)vec_at(&v, 0);
   assert(*elem == 42);
   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_set_get()
{
   Vec v;
   int a = 7;
   vec_new(&v, sizeof(int), NULL);
   vec_push(&v, NULL);
   vec_set(&v, &a, 0);
   int b = 0;
   vec_get(&v, 0, &b);
   assert(b == 7);
   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_insert_remove()
{
   Vec v;
   vec_new(&v, sizeof(int), NULL);
   int x = 10, y = 20, z = 30;
   vec_push(&v, &x);
   vec_push(&v, &z);
   vec_insert(&v, 1, &y);  // Insert y between x and z

   int arr[3];
   for (int i = 0; i < 3; i++)
      vec_get(&v, i, &arr[i]);
   assert(arr[0] == 10 && arr[1] == 20 && arr[2] == 30);

   int removed;
   vec_remove(&v, 1, &removed);  // Remove y
   assert(removed == 20);

   vec_get(&v, 1, &removed);
   assert(removed == 30);

   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_pop()
{
   Vec v;
   vec_new(&v, sizeof(int), NULL);
   int a = 100;
   vec_push(&v, &a);
   int b;
   vec_pop(&v, &b);
   assert(b == 100);
   assert(vec_is_empty(&v));
   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_truncate_shrink()
{
   Vec v;
   int arr[] = {1, 2, 3, 4};
   vec_from(&v, sizeof(int), arr, 4, NULL);
   vec_truncate(&v, 2);
   assert(v.len == 2);
   int value;
   vec_get(&v, 1, &value);
   assert(value == 2);
   vec_shrink_to_fit(&v);
   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_swap()
{
   typedef struct Big {
      char x[300];
   } Big;
   
   Vec v;
   Big arr[4] = {0};
   memset(&arr[0], 'A', sizeof(Big));
   memset(&arr[1], 'B', sizeof(Big));
   memset(&arr[2], 'C', sizeof(Big));
   memset(&arr[3], 'D', sizeof(Big));
   vec_from(&v, sizeof(Big), arr, 4, NULL);
   vec_swap(&v, 0, 2, 2);
   Big tmp[2];
   memcpy(tmp, &arr[0], sizeof(Big) * 2);
   memcpy(&arr[0], &arr[2], sizeof(Big) * 2);
   memcpy(&arr[2], tmp, sizeof(Big) * 2);
   assert(memcmp(vec_at(&v, 0), &arr[0], sizeof(Big) * 4) == 0);
   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_mem_ops()
{
   Vec v;
   size_t len;
   int arr[] = {9, 8, 7};
   vec_from(&v, sizeof(int), arr, 3, NULL);

   int copy[3];
   vec_memcpy(&v, copy, vec_slice(&v, &len), 3);
   assert(vec_memcmp(&v, copy, arr, 3) == 0);

   int moved[3];
   vec_memmove(&v, moved, copy, 3);
   assert(vec_memcmp(&v, moved, arr, 3) == 0);

   vec_memset(&v, moved, 0, 3);
   for (int i = 0; i < 3; i++)
      assert(moved[i] == 0);

   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

void test_vec_insert_null()
{
   Vec v;
   
   vec_new(&v, sizeof(int), NULL);
   int a = 1;
   vec_insert(&v, 0, &a);
   int *b = (int *)vec_insert(&v, 1, NULL);
   *b = 2;
   int *elem0 = (int *)vec_at(&v, 0);
   int *elem1 = (int *)vec_at(&v, 1);
   assert(*elem0 == 1);
   assert(*elem1 == 2);
   vec_free(&v);
   
   printf("%s passed\n", __func__);
}

typedef struct OwnsMem {
   char *mem;
} OwnsMem;

static int ownsmem_alloc_count = 0;

static void new_ownsmem(OwnsMem *p, const char *str)
{
   ownsmem_alloc_count++;
   p->mem = strdup(str);
}

static void free_ownsmem(OwnsMem *p)
{
   ownsmem_alloc_count--;
   free(p->mem);
}

void test_vec_autofree()
{
   Vec v;
   
   vec_new(&v, sizeof(OwnsMem), (FreeFn)free_ownsmem);
   for (int i = 0; i < 5; i++) {
      OwnsMem om;
      new_ownsmem(&om, "test");
      vec_push(&v, &om);
   }
   assert(ownsmem_alloc_count == 5);
   for (size_t i = 0; i < v.len; i++) {
      if (i % 2) {
         OwnsMem om;
         vec_remove(&v, i, &om);
         assert(strcmp(om.mem, "test") == 0);
         free_ownsmem(&om);
      }
   }
   vec_free(&v);
   assert(ownsmem_alloc_count == 0);
   
   printf("%s passed\n", __func__);
}

int main()
{
   test_vec_new_and_empty();
   test_vec_new_with();
   test_vec_from();
   test_vec_push_and_elem_at();
   test_vec_set_get();
   test_vec_insert_remove();
   test_vec_pop();
   test_vec_truncate_shrink();
   test_vec_swap();
   test_vec_mem_ops();
   test_vec_insert_null();
   test_vec_autofree();
   
   printf("%s suite passed!\n", __FILE__);
   return 0;
}
