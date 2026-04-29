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

#define MIN_LOAD           0.25
#define MAX_LOAD           0.75
#define HMAP_START_BUCKETS 64 /**< initial number of buckets */

/**
 * @brief Perl's hash function
 */
static Hash default_hash_fn(const void *key, size_t size)
{
   register const uint8_t *data = (const uint8_t *)key;
   register size_t         i = size;
   register Hash           hash = 0;  // or seed

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

/**
 * @brief round up to nearest power of two
 */
INLINE static Hash roundup_pow2(Hash num)
{
   size_t shift;

   if (!num)
      return 1;

   num--;
   for (shift = 1; shift < sizeof(num) * 8; shift <<= 1) {
      num |= num >> shift;
   }
   num++;

   return num;
}

//
// MARK: HashNode
//

INLINE static bool hashnode_eq(HashNode *node, const void *key, size_t key_size)
{
   if (node->key_size != key_size)
      return false;
   return !memcmp(node->key, key, node->key_size);
}

INLINE static HashNode *hashnode_new(const void *key, size_t key_size, void *val)
{
   HashNode *node;

   node = (HashNode *)malloc(sizeof(HashNode));
   node->key = key;
   node->key_size = key_size;
   node->val = val;
   node->next = NULL;

   return node;
}

//
// MARK: HashMap
//

INLINE static size_t hashmap_real_key_size(HashMap *map, const void *key)
{
   if (map->base_key_size == HMAP_KEY_SIZE_STR)
      return key ? strlen((const char *)key) : 0;
   return map->base_key_size;
}

INLINE static HashNode *
hashmap_find(HashMap *map, const void *key, size_t base_key_size, Hash hash, HashNode **pprev)
{
   HashNode *node, *prev = NULL;

   for (node = map->buckets[hash]; node; node = node->next) {
      if (hashnode_eq(node, key, base_key_size))
         break;
      prev = node;
   }

   if (pprev)
      *pprev = prev;

   return node;
}

static HashNode *
hashmap_insert(HashMap *map, const void *key, size_t key_size, Hash hash, void *val)
{
   HashNode *node = hashnode_new(key, key_size, val);

   node->next = map->buckets[hash];
   map->buckets[hash] = node;
   map->n_items++;
   hashmap_rehash(map);

   return node;
}

void hashmap_init(HashMap *map, size_t base_key_size, HashFn hash_fn)
{
   map->n_buckets = HMAP_START_BUCKETS;
   map->buckets = (HashNode **)calloc((size_t)map->n_buckets, sizeof(HashNode *));
   map->n_items = 0;
   map->base_key_size = base_key_size;
   map->hash_fn = hash_fn ? hash_fn : default_hash_fn;
}

bool hashmap_remove(HashMap *map, const void *key, void **pval)
{
   size_t    key_size = hashmap_real_key_size(map, key);
   Hash      hash = calc_hash(key, key_size, map->hash_fn, map->n_buckets);
   HashNode *node, *prev;

   node = hashmap_find(map, key, key_size, hash, &prev);
   if (!node)
      return false;

   if (prev)
      prev->next = node->next;
   else
      map->buckets[hash] = node->next;
   map->n_items--;

   if (pval)
      *pval = node->val;
   free(node);

   return true;
}

bool hashmap_get(HashMap *map, const void *key, void **pval)
{
   HashEntry entry;

   if (!hashentry_init(&entry, map, key))
      return false;

   hashentry_val(&entry, pval);

   return true;
}

bool hashmap_set(HashMap *map, const void *key, void *val, void **pval)
{
   HashEntry entry;

   hashentry_init(&entry, map, key);

   return hashentry_set(&entry, val, pval);
}

bool hashmap_contains(const HashMap *map, const void *key)
{
   HashEntry entry;

   return hashentry_init(&entry, (HashMap *)map, key);
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
   float load = (float)map->n_items / (float)map->n_buckets;

   if (load < MIN_LOAD || load > MAX_LOAD) {
      HashNode **buckets;
      Hash       n_buckets;

      n_buckets = (Hash)((float)(map->n_items * 2) / (MIN_LOAD + MAX_LOAD));
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
}

//
// MARK: HashEntry
//
bool hashentry_init(HashEntry *entry, HashMap *map, const void *key)
{
   size_t key_size = hashmap_real_key_size(map, key);
   Hash   hash = calc_hash(key, key_size, map->hash_fn, map->n_buckets);

   entry->map = map;
   entry->node = hashmap_find(map, key, key_size, hash, NULL);
   entry->key = key;
   entry->key_size = key_size;
   entry->hash = hash;

   return entry->node != NULL;
}

bool hashentry_set(HashEntry *entry, void *val, void **pval)
{
   HashMap *map = entry->map;
   bool     found;

   if (entry->node) {
      found = true;
      if (pval)
         *pval = entry->node->val;
      entry->node->val = val;
   }
   else {
      found = false;
      if (pval)
         *pval = NULL;
      entry->node = hashmap_insert(map, entry->key, entry->key_size, entry->hash, val);
   }

   return found;
}

//
// MARK: HashIter
//

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