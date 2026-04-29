/*
 * Copyright (c) 2026 Alessandro Martone
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "hashmap.h"

/**
 * @brief initial n_buckets
 */
#define INITIAL_N_BUCKETS 8

/**
 * @brief Perl's hash function
 */
static Hash default_hash_fn(const void *key, size_t size)
{
   register const uint8_t *data;
   register size_t         i;
   register Hash           hash;

   data = (const uint8_t *)key;
   i = size;
   hash = 0;  // or seed

   while (i--) {
      hash += *data++;
      hash += (hash << 10);
      hash ^= (hash >> 6);
   }

   hash += (hash << 3);
   hash ^= (hash >> 11);
   hash += (hash << 15);

   return hash;
}

INLINE static Hash calc_hash(const void *key, size_t key_size, HashFn hash_fn, Hash n_buckets)
{
   Hash hash = hash_fn(key, key_size);

   return hash & (n_buckets - 1);
}

//
// MARK: HashNode
//
INLINE static bool hashnode_eq(HashNode *node, const void *key, size_t real_size)
{
   if (node->key_size != real_size)
      return false;
   if (node->key == key)
      return true;
   return !memcmp(node->key, key, node->key_size);
}

INLINE static HashNode *hashnode_new(const void *key, size_t key_size, void *value)
{
   HashNode *node = (HashNode *)malloc(sizeof(HashNode));

   node->key = key;
   node->key_size = key_size;
   node->value = value;
   node->next = NULL;

   return node;
}

//
// MARK: HashMap
//

/**
 * @brief round up to nearest power of two
 */
INLINE static Hash roundup_pow2(Hash num)
{
   uint8_t i;

   if (!num)
      return 1;

   // a quick test shows that -O2 unrolls the loop on sizeof(num) == 4, but not 8. -O3 does it in both cases
   num--;
   for (i = 0; i < sizeof(num); i++) {
      num |= num >> (1 << i);
   }
   num++;

   return num;
}

INLINE static size_t hashmap_key_size(HashMap *map, const void *key)
{
   if (map->base_key_size == KEY_SIZE_STR)
      return key ? strlen((const char *)key) : 0;
   return map->base_key_size;
}

INLINE static float hashmap_load(HashMap *map)
{
   return (float)map->n_items / (float)map->n_buckets;
}

static void hashmap_rehash_inner(HashMap *map)
{
   HashNode **buckets;
   Hash       n_buckets;

   n_buckets = (Hash)((float)(map->n_items * 2) / (map->min_load + map->max_load));
   n_buckets = roundup_pow2(n_buckets);
   if (n_buckets == map->n_buckets)
      return;

   buckets = (HashNode **)calloc((size_t)n_buckets, sizeof(HashNode *));

   while (map->n_buckets--) {
      HashNode *node = map->buckets[map->n_buckets];
      while (node) {
         HashNode *next = node->next;
         Hash      hash = calc_hash(node->key, node->key_size, map->hash_fn, n_buckets);
         node->next = buckets[hash];
         buckets[hash] = node;
         node = next;
      }
   }

   free(map->buckets);
   map->n_buckets = n_buckets;
   map->buckets = buckets;
}

INLINE static HashNode *hashmap_find(HashMap *map, const void *key, size_t key_size, Hash hash)
{
   HashNode *node;

   for (node = map->buckets[hash]; node; node = node->next) {
      if (hashnode_eq(node, key, key_size))
         break;
   }

   return node;
}

void hashmap_init_with(HashMap *map, size_t base_key_size, HashFn hash_fn, Hash n_buckets)
{
   map->n_buckets = roundup_pow2(n_buckets);
   map->buckets = (HashNode **)calloc((size_t)map->n_buckets, sizeof(HashNode *));
   map->n_items = 0;
   map->base_key_size = base_key_size;
   map->hash_fn = hash_fn ? hash_fn : default_hash_fn;
   map->min_load = 0.25;
   map->max_load = 0.75;
}

void hashmap_init(HashMap *map, size_t key_size, HashFn hash_fn)
{
   hashmap_init_with(map, key_size, hash_fn, INITIAL_N_BUCKETS);
}

void hashmap_insert(HashMap *map, const void *key, void *value)
{
   HashEntry entry;

   hashentry_init(&entry, map, key);
   hashentry_set(&entry, value, NULL);
}

bool hashmap_remove(HashMap *map, const void *key, void **pvalue)
{
   size_t    key_size = hashmap_key_size(map, key);
   Hash      hash = calc_hash(key, key_size, map->hash_fn, map->n_buckets);
   HashNode *node, *prev = NULL;

   for (node = map->buckets[hash]; node; node = node->next) {
      if (hashnode_eq(node, key, key_size))
         break;
      prev = node;
   }

   if (!node)
      return false;

   if (prev)
      prev->next = node->next;
   else
      map->buckets[hash] = node->next;
   map->n_items--;

   if (pvalue)
      *pvalue = node->value;
   free(node);

   return true;
}

bool hashmap_get(HashMap *map, const void *key, void **pvalue)
{
   HashEntry entry;

   if (!hashentry_init(&entry, map, key))
      return false;

   hashentry_value(&entry, pvalue);

   return true;
}

bool hashmap_contains(const HashMap *map, const void *key)
{
   HashEntry entry;
   return hashentry_init(&entry, (HashMap *)map, key);
}

void hashmap_clear(HashMap *map)
{
   size_t i;

   for (i = 0; i < map->n_buckets; i++) {
      if (map->buckets[i]) {
         free(map->buckets[i]);
         map->buckets[i] = NULL;
      }
   }
   map->n_items = 0;
}

void hashmap_free(HashMap *map)
{
   while (map->n_buckets--) {
      if (map->buckets[map->n_buckets])
         free(map->buckets[map->n_buckets]);
   }
   free(map->buckets);
   memset(map, 0, sizeof(*map));
}

void hashmap_rehash(HashMap *map)
{
   float load = hashmap_load(map);
   if (load < map->min_load || load > map->max_load)
      hashmap_rehash_inner(map);
}

bool hashmap_merge(HashMap *dst, HashMap *src)
{
   HashIter iter;

   if (dst->base_key_size != src->base_key_size)
      return false;

   hashiter_init(src, &iter);
   while (hashiter_next(&iter)) {
      hashmap_insert(dst, hashiter_key(&iter), hashiter_value(&iter));
   }
   hashmap_free(src);

   return true;
}

bool hashmap_set_thresholds(HashMap *map, float min_load, float max_load)
{
   if (min_load <= 0. || max_load <= 0. || min_load > max_load)
      return false;

   map->min_load = min_load;
   map->max_load = max_load;

   return true;
}

//
// MARK: HashEntry
//
bool hashentry_init(HashEntry *entry, HashMap *map, const void *key)
{
   size_t key_size = hashmap_key_size(map, key);
   Hash   hash = calc_hash(key, key_size, map->hash_fn, map->n_buckets);

   entry->map = map;
   entry->node = hashmap_find(map, key, key_size, hash);
   entry->key = key;
   entry->key_size = key_size;
   entry->hash = hash;

   return entry->node != NULL;
}

bool hashentry_set(HashEntry *entry, void *value, void **pvalue)
{
   HashMap  *map = entry->map;
   HashNode *node = entry->node;
   bool      found;

   if (!node) {
      found = false;
      if (pvalue)
         *pvalue = NULL;

      node = hashnode_new(entry->key, entry->key_size, value);
      node->next = map->buckets[entry->hash];
      map->buckets[entry->hash] = node;
      map->n_items++;

      hashmap_rehash(map);
   }
   else {
      found = true;
      if (pvalue)
         *pvalue = node->value;
      node->value = value;
   }

   entry->node = node;

   return found;
}

//
// MARK: HashIter
//
void hashiter_init(const HashMap *map, HashIter *iter)
{
   iter->map = map;
   iter->node = NULL;
   iter->hash = (Hash)-1;
}

bool hashiter_next(HashIter *iter)
{
   const HashMap *map = iter->map;

   if (iter->node)
      iter->node = iter->node->next;

   while (!iter->node) {
      if (++iter->hash >= map->n_buckets)
         return false;

      iter->node = map->buckets[iter->hash];
   }

   return true;
}

/**
 * TODO:
 * - optimize hashmap_merge
 * - is hashmap_clear necessary?
 */