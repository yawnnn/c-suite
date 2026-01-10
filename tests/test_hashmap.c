#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hashmap.h"

static Hash int_hash(const void *key, size_t size)
{
   (void)size;
   return *(int *)key;
}

static void test_insert_get_contains(void)
{
   HashMap hm;
   hashmap_init(&hm, sizeof(int), int_hash);

   int   k = 42;
   int   v = 1337;
   void *out = NULL;

   assert(!hashmap_contains(&hm, &k));

   hashmap_insert(&hm, &k, &v);

   assert(hashmap_contains(&hm, &k));
   assert(hashmap_get(&hm, &k, &out));
   assert(out == &v);

   hashmap_free(&hm);

   printf("%s passed\n", __func__);
}

static void test_overwrite_value(void)
{
   HashMap hm;
   hashmap_init(&hm, sizeof(int), int_hash);

   int   k = 1;
   int   v1 = 10;
   int   v2 = 20;
   void *out = NULL;

   hashmap_insert(&hm, &k, &v1);
   hashmap_insert(&hm, &k, &v2);

   assert(hashmap_get(&hm, &k, &out));
   assert(out == &v2);

   hashmap_free(&hm);

   printf("%s passed\n", __func__);
}

static void test_remove(void)
{
   HashMap hm;
   hashmap_init(&hm, sizeof(int), int_hash);

   int   k = 5;
   int   v = 99;
   void *out = NULL;

   hashmap_insert(&hm, &k, &v);

   assert(hashmap_remove(&hm, &k, &out));
   assert(out == &v);
   assert(!hashmap_contains(&hm, &k));
   assert(!hashmap_remove(&hm, &k, NULL));

   hashmap_free(&hm);

   printf("%s passed\n", __func__);
}

static void test_clear(void)
{
   HashMap hm;
   hashmap_init(&hm, sizeof(int), int_hash);

   for (int i = 0; i < 100; i++)
      hashmap_insert(&hm, &i, &i);

   assert(hashmap_len(&hm));

   hashmap_clear(&hm);

   assert(!hashmap_len(&hm));

   hashmap_free(&hm);

   printf("%s passed\n", __func__);
}

static void test_string_keys(void)
{
   HashMap hm;
   hashmap_init(&hm, KEY_SIZE_STR, NULL);

   hashmap_insert(&hm, "hello", "world");
   hashmap_insert(&hm, "foo", "bar");

   void *out = NULL;
   assert(hashmap_get(&hm, "hello", &out));
   assert(strcmp(out, "world") == 0);

   assert(hashmap_contains(&hm, "foo"));
   assert(!hashmap_contains(&hm, "baz"));

   hashmap_free(&hm);

   printf("%s passed\n", __func__);
}

static void test_hashentry(void)
{
   HashMap hm;
   hashmap_init(&hm, sizeof(int), int_hash);

   int k = 7;
   int v = 70;
   hashmap_insert(&hm, &k, &v);

   HashEntry he;
   assert(hashentry_init(&he, &hm, &k));
   assert(hashentry_is_occupied(&he));

   void *out = NULL;
   assert(hashentry_value(&he, &out));
   assert(out == &v);

   int v2 = 71;
   out = NULL;
   assert(hashentry_set(&he, &v2, &out));
   assert(out == &v);

   assert(hashmap_get(&hm, &k, &out));
   assert(out == &v2);

   hashmap_free(&hm);

   printf("%s passed\n", __func__);
}

static void test_iteration(void)
{
   HashMap hm;
   hashmap_init(&hm, sizeof(int), int_hash);

   int keys[5] = {1, 2, 3, 4, 5};

   for (int i = 0; i < 5; i++)
      hashmap_insert(&hm, &keys[i], &keys[i]);

   bool seen[5] = {false};

   HashIter it;
   hashiter_init(&hm, &it);

   while (hashiter_next(&it)) {
      int k = *(int *)hashiter_key(&it);
      assert(k >= 1 && k <= 5);
      seen[k - 1] = true;
   }

   for (int i = 0; i < 5; i++)
      assert(seen[i]);

   hashmap_free(&hm);

   printf("%s passed\n", __func__);
}

static void test_merge(void)
{
   HashMap a, b;
   hashmap_init(&a, sizeof(int), int_hash);
   hashmap_init(&b, sizeof(int), int_hash);

   int k1 = 1, k2 = 2, k3 = 3;

   hashmap_insert(&a, &k1, &k1);
   hashmap_insert(&b, &k2, &k2);
   hashmap_insert(&b, &k3, &k3);

   assert(hashmap_merge(&a, &b));

   assert(hashmap_contains(&a, &k1));
   assert(hashmap_contains(&a, &k2));
   assert(hashmap_contains(&a, &k3));

   assert(!hashmap_len(&b));

   hashmap_free(&a);
   hashmap_free(&b);

   printf("%s passed\n", __func__);
}

int main(void)
{
   test_insert_get_contains();
   test_overwrite_value();
   test_remove();
   test_clear();
   test_string_keys();
   test_hashentry();
   test_iteration();
   test_merge();

   printf("%s suite passed!\n", __FILE__);
   return 0;
}
