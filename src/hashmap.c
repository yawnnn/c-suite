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

#define MIN_LOAD      0.25
#define MAX_LOAD      0.75
#define START_BUCKETS 64 /**< initial number of buckets */

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

INLINE static Hash bucket_idx(Hash hash, Hash n_buckets)
{
   return hash & (n_buckets - 1);
}

//
// MARK: HashNode
//

static HashNode *
hashnode_new(const void *key, size_t key_size, Hash hash, const void *val, size_t val_size)
{
   HashNode *node = malloc((size_t)ALIGN_UP(sizeof(HashNode)) + key_size);

   node->next = NULL;
   node->val = malloc(val_size);
   memcpy(node->val, val, val_size);
   node->val_size = (uint32_t)val_size;
   node->hash = hash;
   memcpy(HASHNODE_KEY(node), key, key_size);

   return node;
}

INLINE static void hashnode_free(HashNode *node)
{
   free(node->val);
   free(node);
}

static bool hashnode_eq(HashNode *node, const void *key, size_t key_size, Hash hash)
{
   return node->hash == hash && !memcmp(HASHNODE_KEY(node), key, key_size);
}

//
// MARK: HashMap
//

INLINE static size_t hashmap_key_size(HashMap *map, const void *key)
{
   return map->base_key_size == HASHMAP_LEN_STR ? strlen((char *)key) + 1 : map->base_key_size;
}

INLINE static size_t hashmap_val_size(HashMap *map, const void *val)
{
   return map->base_val_size == HASHMAP_LEN_STR ? strlen((char *)val) + 1 : map->base_val_size;
}

static HashNode *
hashmap_find(HashMap *map, const void *key, size_t key_size, Hash hash, HashNode **pprev)
{
   HashNode *node, *prev = NULL;
   Hash      idx = bucket_idx(hash, map->n_buckets);

   if (!map->n_buckets)
      return NULL;

   for (node = map->buckets[idx]; node; node = node->next) {
      if (hashnode_eq(node, key, key_size, hash))
         return node;
      prev = node;
   }

   if (pprev)
      *pprev = prev;

   return NULL;
}

static HashNode *hashmap_insert(
   HashMap    *map,
   const void *key,
   size_t      key_size,
   Hash        hash,
   const void *val,
   size_t      val_size,
   HashNode  **pprev
)
{
   HashNode *node = hashnode_new(key, key_size, hash, val, val_size);
   HashNode *prev = NULL;
   Hash      idx;

   if (!map->n_buckets) {
      map->n_buckets = START_BUCKETS;
      map->buckets = calloc(map->n_buckets, sizeof(HashNode *));
   }

   idx = bucket_idx(hash, map->n_buckets);
   node->next = map->buckets[idx];
   map->buckets[idx] = node;
   map->n_items++;

   if (hashmap_rehash(map)) {
      HashNode *tmp = map->buckets[bucket_idx(hash, map->n_buckets)];
      while (tmp != node) {
         prev = tmp;
         tmp = tmp->next;
      }
   }

   *pprev = prev;

   return node;
}

void hashmap_new(HashMap *map, size_t base_key_size, size_t base_val_size, HashFn hash_fn)
{
   memset(map, 0, sizeof(*map));
   map->base_key_size = base_key_size;
   map->base_val_size = base_val_size;
   map->hash_fn = hash_fn ? hash_fn : default_hash_fn;
}

bool hashmap_rehash(HashMap *map)
{
   HashNode **buckets;
   Hash       n_buckets;
   float      load;

   if (!map->n_buckets)
      return false;

   load = (float)map->n_items / (float)map->n_buckets;
   if (load >= MIN_LOAD && load <= MAX_LOAD)
      return false;

   n_buckets = (Hash)((float)(map->n_items * 2) / (MIN_LOAD + MAX_LOAD));
   n_buckets = roundup_pow2(n_buckets);
   buckets = (HashNode **)calloc((size_t)n_buckets, sizeof(HashNode *));

   while (map->n_buckets--) {
      HashNode *node = map->buckets[map->n_buckets];

      while (node) {
         HashNode *next = node->next;
         Hash      idx = bucket_idx(node->hash, n_buckets);

         node->next = buckets[idx];
         buckets[idx] = node;
         node = next;
      }
   }

   free(map->buckets);
   map->buckets = buckets;
   map->n_buckets = n_buckets;

   return true;
}

void hashmap_free(HashMap *map)
{
   while (map->n_buckets--) {
      HashNode *node = map->buckets[map->n_buckets];
      while (node) {
         HashNode *next = node->next;
         hashnode_free(node);
         node = next;
      }
   }
   free(map->buckets);
   map->buckets = NULL;
   map->n_buckets = map->n_items = 0;
}

//
// MARK: HashEntry
//
bool hashentry_init(HashEntry *entry, HashMap *map, const void *key)
{
   entry->map = map;
   entry->key = key;
   entry->key_size = hashmap_key_size(map, key);
   entry->hash = map->hash_fn(key, entry->key_size);
   entry->node = hashmap_find(map, key, entry->key_size, entry->hash, &entry->prev);

   return entry->node != NULL;
}

bool hashentry_set(HashEntry *entry, const void *val, void **pval, size_t *pval_size)
{
   HashMap *map = entry->map;
   Hash     val_size = hashmap_val_size(map, val);
   bool     found = entry->node != NULL;

   if (found) {
      HashNode *node = entry->node;

      if (pval_size)
         *pval_size = node->val_size;
      if (pval) {
         *pval = node->val;
         node->val = malloc(val_size);
         node->val_size = val_size;
      }
      else if (node->val_size < val_size) {
         node->val = realloc(node->val, val_size);
         node->val_size = val_size;
      }
      memcpy(node->val, val, val_size);
   }
   else {
      if (pval)
         *pval = NULL;
      if (pval_size)
         *pval_size = 0;
      entry->node =
         hashmap_insert(map, entry->key, entry->key_size, entry->hash, val, val_size, &entry->prev);
   }

   return found;
}

bool hashentry_remove(HashEntry *entry, void **pval, size_t *pval_size)
{
   HashMap  *map = entry->map;
   HashNode *node = entry->node, *prev = entry->prev;

   if (!node) {
      if (pval)
         *pval = NULL;
      if (pval_size)
         *pval_size = 0;
      return false;
   }

   if (prev)
      prev->next = node->next;
   else
      map->buckets[bucket_idx(entry->hash, map->n_buckets)] = node->next;
   map->n_items--;

   if (pval_size)
      *pval_size = node->val_size;
   if (pval) {
      *pval = node->val;
      free(node);
   }
   else
      hashnode_free(node);
   entry->node = entry->prev = NULL;

   return true;
}

//
// MARK: HashIter
//

bool hashiter_next(HashIter *iter)
{
   const HashMap *map = iter->map;

   if (iter->node && iter->node->next) {
      iter->node = iter->node->next;
      return true;
   }

   while (++iter->idx < map->n_buckets) {
      if (map->buckets[iter->idx]) {
         iter->node = map->buckets[iter->idx];
         return true;
      }
   }

   return false;
}