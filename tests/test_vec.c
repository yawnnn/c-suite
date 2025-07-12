#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "vec.h"

void test_vec_new_and_empty()
{
   Vec v;
   vec_new(&v, sizeof(int));
   assert(vec_data(&v) == NULL);
   assert(vec_is_empty(&v));
   vec_free(&v);
   printf("%s passed\n", __func__);
}

void test_vec_new_with()
{
   Vec v;
   vec_new_with(&v, sizeof(int), 10);
   assert(vec_data(&v) == NULL);  // Still 0-length
   vec_free(&v);
   printf("%s passed\n", __func__);
}

void test_vec_new_with_zeroed()
{
   Vec v;
   vec_new_with_zeroed(&v, sizeof(int), 4);
   int *data = (int *)vec_data(&v);
   for (int i = 0; i < 4; i++)
      assert(data[i] == 0);
   vec_free(&v);
   printf("%s passed\n", __func__);
}

void test_vec_from()
{
   int arr[] = {1, 2, 3};
   Vec v;
   vec_from(&v, sizeof(int), arr, 3);
   assert(v.len == 3);
   int *data = (int *)vec_data(&v);
   for (int i = 0; i < 3; i++)
      assert(data[i] == arr[i]);
   vec_free(&v);
   printf("%s passed\n", __func__);
}

void test_vec_push_and_elem_at()
{
   Vec v;
   vec_new(&v, sizeof(int));
   int a = 42;
   vec_push(&v, &a);
   assert(v.len == 1);
   int *elem = (int *)vec_elem_at(&v, 0);
   assert(*elem == 42);
   vec_free(&v);
   printf("%s passed\n", __func__);
}

void test_vec_set_get()
{
   Vec v;
   int a = 7;
   vec_new_with_zeroed(&v, sizeof(int), 1);
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
   vec_new(&v, sizeof(int));
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
   vec_new(&v, sizeof(int));
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
   vec_from(&v, sizeof(int), arr, 4);
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
   Vec v;
   int arr[] = {1, 2};
   vec_from(&v, sizeof(int), arr, 2);
   int tmp;
   vec_swap(&v, 0, 1, &tmp);
   int a, b;
   vec_get(&v, 0, &a);
   vec_get(&v, 1, &b);
   assert(a == 2 && b == 1);
   vec_free(&v);
   printf("%s passed\n", __func__);
}

void test_vec_mem_ops()
{
   Vec v;
   int arr[] = {9, 8, 7};
   vec_from(&v, sizeof(int), arr, 3);

   int copy[3];
   vec_memcpy(&v, copy, vec_data(&v), 3);
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

int main()
{
   test_vec_new_and_empty();
   test_vec_new_with();
   test_vec_new_with_zeroed();
   test_vec_from();
   test_vec_push_and_elem_at();
   test_vec_set_get();
   test_vec_insert_remove();
   test_vec_pop();
   test_vec_truncate_shrink();
   test_vec_swap();
   test_vec_mem_ops();
   printf("All Vec tests passed!\n");
   return 0;
}
