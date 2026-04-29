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

   hashmap_set(&map, &k, &v, NULL);

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

   hashmap_set(&map, &k, &v1, NULL);
   hashmap_set(&map, &k, &v2, NULL);

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

   hashmap_set(&map, &k, &v, NULL);

   assert(hashmap_remove(&map, &k, &out));
   assert(out == &v);
   assert(!hashmap_contains(&map, &k));
   assert(!hashmap_remove(&map, &k, NULL));

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_string_keys(void)
{
   HashMap map;
   hashmap_init(&map, HMAP_KEY_SIZE_STR, NULL);

   hashmap_set(&map, "hello", "world", NULL);
   hashmap_set(&map, "foo", "bar", NULL);

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
   hashmap_set(&map, &k, &v, NULL);

   HashEntry entry;
   assert(hashentry_init(&entry, &map, &k));
   assert(hashentry_is_occupied(&entry));

   void *out = NULL;
   assert(hashentry_val(&entry, &out));
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
      hashmap_set(&map, &keys[i], &keys[i], NULL);

   bool seen[5] = {false};

   HashIter iter;
   hashiter_init(&iter, &map);

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

int main(void)
{
   test_insert_get_contains();
   test_overwrite_value();
   test_remove();
   test_string_keys();
   test_hashentry();
   test_iteration();

   printf("%s suite passed!\n", __FILE__);
   return 0;
}
