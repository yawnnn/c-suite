#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hashmap.h"

static void test_insert_get_contains(void)
{
   HashMap map;
   hashmap_new(&map, sizeof(int), sizeof(int), NULL);

   int k = 42;
   int v = 1337;

   assert(!hashmap_contains(&map, &k));

   hashmap_set(&map, &k, &v, NULL, NULL);

   assert(hashmap_contains(&map, &k));

   const int *out = hashmap_get(&map, &k, NULL);
   assert(out != NULL);
   assert(*out == v);

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_overwrite_value(void)
{
   HashMap map;
   hashmap_new(&map, sizeof(int), sizeof(int), NULL);

   int k = 1;
   int v1 = 10;
   int v2 = 20;
   const int *out = NULL;

   hashmap_set(&map, &k, &v1, NULL, NULL);
   hashmap_set(&map, &k, &v2, NULL, NULL);

   out = hashmap_get(&map, &k, NULL);
   assert(out != NULL);
   assert(*out == v2);

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_remove(void)
{
   HashMap map;
   hashmap_new(&map, sizeof(int), sizeof(int), NULL);

   int   k = 5;
   int   v = 99;
   void *out = NULL;

   hashmap_set(&map, &k, &v, NULL, NULL);

   assert(hashmap_remove(&map, &k, &out, NULL));
   assert(out != NULL);
   assert(*(int *)out == v);

   free(out);

   assert(!hashmap_contains(&map, &k));
   assert(!hashmap_remove(&map, &k, NULL, NULL));

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_string_keys(void)
{
   HashMap map;
   hashmap_new(&map, HASHMAP_LEN_STR, HASHMAP_LEN_STR, NULL);

   hashmap_set(&map, "hello", "world", NULL, NULL);
   hashmap_set(&map, "foo", "bar", NULL, NULL);

   const char *out = hashmap_get(&map, "hello", NULL);
   assert(out != NULL);
   assert(strcmp(out, "world") == 0);

   assert(hashmap_contains(&map, "foo"));
   assert(!hashmap_contains(&map, "baz"));

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_hashentry(void)
{
   HashMap map;
   hashmap_new(&map, sizeof(int), sizeof(int), NULL);

   int k = 7;
   int v = 70;
   hashmap_set(&map, &k, &v, NULL, NULL);

   HashEntry entry;
   assert(hashentry_init(&entry, &map, &k));
   assert(hashentry_found(&entry));

   const int *out = hashentry_val(&entry, NULL);
   assert(out != NULL);
   assert(*out == v);

   int   v2 = 71;
   void *old = NULL;

   assert(hashentry_set(&entry, &v2, &old, NULL));
   assert(old != NULL);
   assert(*(int *)old == v);

   free(old);

   const int *newv = hashmap_get(&map, &k, NULL);
   assert(newv != NULL);
   assert(*newv == v2);

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_iteration(void)
{
   HashMap map;
   hashmap_new(&map, sizeof(int), sizeof(int), NULL);

   int keys[5] = {1, 2, 3, 4, 5};

   for (int i = 0; i < 5; i++)
      hashmap_set(&map, &keys[i], &keys[i], NULL, NULL);

   bool seen[5] = {false};

   HashIter iter;
   hashiter_init(&iter, &map);

   while (hashiter_next(&iter)) {
      int k = *(int *)hashiter_key(&iter);
      int v = *(int *)hashiter_val(&iter, NULL);

      assert(k >= 1 && k <= 5);
      assert(v == k);  // NEW: also validate value
      seen[k - 1] = true;
   }

   for (int i = 0; i < 5; i++)
      assert(seen[i]);

   hashmap_free(&map);

   printf("%s passed\n", __func__);
}

static void test_len(void)
{
   HashMap map;
   hashmap_new(&map, sizeof(int), sizeof(int), NULL);

   int k = 1, v = 1;
   assert(hashmap_len(&map) == 0);

   hashmap_set(&map, &k, &v, NULL, NULL);
   assert(hashmap_len(&map) == 1);

   hashmap_set(&map, &k, &v, NULL, NULL);
   assert(hashmap_len(&map) == 1);

   hashmap_remove(&map, &k, NULL, NULL);
   assert(hashmap_len(&map) == 0);

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
   test_len();

   printf("%s suite passed!\n", __FILE__);
   return 0;
}