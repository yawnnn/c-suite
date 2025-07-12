#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llist.h"

static void test_init_free()
{
   LList list;
   llist_init(&list);
   assert(llist_is_empty(&list));
   llist_free(&list);
   printf("%s passed\n", __func__);
}

static void test_push_pop_front_back()
{
   LList list;
   llist_init(&list);

   int a = 1, b = 2, c = 3;
   llist_push_back(&list, &a);
   llist_push_back(&list, &b);
   llist_push_front(&list, &c);

   assert(llist_len(&list) == 3);

   int *out = llist_pop_front(&list);
   assert(out == &c);
   out = llist_pop_back(&list);
   assert(out == &b);
   out = llist_pop_back(&list);
   assert(out == &a);

   assert(llist_is_empty(&list));
   llist_free(&list);
   printf("%s passed\n", __func__);
}

static void test_remove()
{
   LList list;
   llist_init(&list);

   int    a = 10, b = 20, c = 30;
   LNode *n1 = llist_push_back(&list, &a);
   LNode *n2 = llist_push_back(&list, &b);
   LNode *n3 = llist_push_back(&list, &c);

   assert(llist_len(&list) == 3);

   int *out = llist_remove(&list, n2);
   assert(out == &b);
   assert(llist_len(&list) == 2);
   assert(llist_get_at(&list, 0) == n1);
   assert(llist_get_at(&list, 1) == n3);

   llist_free(&list);
   printf("%s passed\n", __func__);
}

static void test_split_join()
{
   LList list, second;
   llist_init(&list);
   llist_init(&second);

   int data[6] = {1, 2, 3, 4, 5, 6};
   for (int i = 0; i < 6; i++)
      llist_push_back(&list, &data[i]);

   LNode *where = llist_get_at(&list, 3);
   llist_split(&list, &second, where);

   assert(llist_len(&list) == 3);
   assert(llist_len(&second) == 3);
   assert(llist_get_at(&list, 2)->data == &data[2]);
   assert(llist_get_at(&second, 0)->data == &data[3]);

   llist_join(&list, &second);
   assert(llist_len(&list) == 6);
   assert(llist_is_empty(&second));

   for (int i = 0; i < 6; i++)
      assert(llist_get_at(&list, i)->data == &data[i]);

   llist_free(&list);
   printf("%s passed\n", __func__);
}

static void test_swap()
{
   LList list;
   llist_init(&list);

   int    a = 1, b = 2;
   LNode *n1 = llist_push_back(&list, &a);
   LNode *n2 = llist_push_back(&list, &b);

   llist_swap(n1, n2);

   assert(n1->data == &b);
   assert(n2->data == &a);

   llist_free(&list);
   printf("%s passed\n", __func__);
}

static void test_free_with()
{
   LList list;
   llist_init(&list);

   char *str1 = strdup("hello");
   char *str2 = strdup("world");

   llist_push_back(&list, str1);
   llist_push_back(&list, str2);

   llist_free_with(&list, free);
   printf("%s passed\n", __func__);
}

static void test_get_at()
{
   LList list;
   llist_init(&list);

   int values[4] = {9, 8, 7, 6};
   for (int i = 0; i < 4; i++)
      llist_push_back(&list, &values[i]);

   for (int i = 0; i < 4; i++)
      assert(llist_get_at(&list, i)->data == &values[i]);

   assert(llist_get_at(&list, 4) == NULL);  // out of bounds

   llist_free(&list);
   printf("%s passed\n", __func__);
}

int main()
{
   test_init_free();
   test_push_pop_front_back();
   test_remove();
   test_split_join();
   test_swap();
   test_free_with();
   test_get_at();
   printf("All tests passed.\n");
   return 0;
}
