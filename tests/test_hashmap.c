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
   HashMap map;
   hashmap_init(&map, sizeof(int), int_hash);

   int   k = 42;
   int   v = 1337;
   void *out = NULL;

   assert(!hashmap_contains(&map, &k));

   hashmap_insert(&map, &k, &v);

   assert(hashmap_contains(&map, &k));
   assert(hashmap_get(&map, &k, &out));
   assert(out == &v);

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_overwrite_value(void)
{
   HashMap map;
   hashmap_init(&map, sizeof(int), int_hash);

   int   k = 1;
   int   v1 = 10;
   int   v2 = 20;
   void *out = NULL;

   hashmap_insert(&map, &k, &v1);
   hashmap_insert(&map, &k, &v2);

   assert(hashmap_get(&map, &k, &out));
   assert(out == &v2);

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_remove(void)
{
   HashMap map;
   hashmap_init(&map, sizeof(int), int_hash);

   int   k = 5;
   int   v = 99;
   void *out = NULL;

   hashmap_insert(&map, &k, &v);

   assert(hashmap_remove(&map, &k, &out));
   assert(out == &v);
   assert(!hashmap_contains(&map, &k));
   assert(!hashmap_remove(&map, &k, NULL));

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_clear(void)
{
   HashMap map;
   hashmap_init(&map, sizeof(int), int_hash);

   for (int i = 0; i < 100; i++)
      hashmap_insert(&map, &i, &i);

   assert(hashmap_len(&map));

   hashmap_clear(&map);

   assert(!hashmap_len(&map));

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_string_keys(void)
{
   HashMap map;
   hashmap_init(&map, KEY_SIZE_STR, NULL);

   hashmap_insert(&map, "hello", "world");
   hashmap_insert(&map, "foo", "bar");

   void *out = NULL;
   assert(hashmap_get(&map, "hello", &out));
   assert(strcmp(out, "world") == 0);

   assert(hashmap_contains(&map, "foo"));
   assert(!hashmap_contains(&map, "baz"));

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_hashentry(void)
{
   HashMap map;
   hashmap_init(&map, sizeof(int), int_hash);

   int k = 7;
   int v = 70;
   hashmap_insert(&map, &k, &v);

   HashEntry entry;
   assert(hashentry_init(&entry, &map, &k));
   assert(hashentry_is_occupied(&entry));

   void *out = NULL;
   assert(hashentry_value(&entry, &out));
   assert(out == &v);

   int v2 = 71;
   out = NULL;
   assert(hashentry_set(&entry, &v2, &out));
   assert(out == &v);

   assert(hashmap_get(&map, &k, &out));
   assert(out == &v2);

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_iteration(void)
{
   HashMap map;
   hashmap_init(&map, sizeof(int), int_hash);

   int keys[5] = {1, 2, 3, 4, 5};

   for (int i = 0; i < 5; i++)
      hashmap_insert(&map, &keys[i], &keys[i]);

   bool seen[5] = {false};

   HashIter iter;
   hashiter_init(&map, &iter);

   while (hashiter_next(&iter)) {
      int k = *(int *)hashiter_key(&iter);
      assert(k >= 1 && k <= 5);
      seen[k - 1] = true;
   }

   for (int i = 0; i < 5; i++)
      assert(seen[i]);

   hashmap_free(&map);

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
