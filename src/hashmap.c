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
INLINE static bool hashnode_eq(HashNode *hnode, const void *key, size_t real_size)
{
   if (hnode->key_size != real_size)
      return false;
   if (hnode->key == key)
      return true;
   return !memcmp(hnode->key, key, hnode->key_size);
}

INLINE static HashNode *hashnode_new(const void *key, size_t key_size, void *value)
{
   HashNode *hnode = (HashNode *)malloc(sizeof(HashNode));

   hnode->key = key;
   hnode->key_size = key_size;
   hnode->value = value;
   hnode->next = NULL;

   return hnode;
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

INLINE static size_t hashmap_key_size(HashMap *hmap, const void *key)
{
   if (hmap->key_size == KEY_SIZE_STR)
      return key ? strlen((const char *)key) : 0;
   return hmap->key_size;
}

INLINE static float hashmap_load_factor(HashMap *hmap)
{
   return (float)hmap->n_items / (float)hmap->n_buckets;
}

static void hashmap_rehash_inner(HashMap *hmap)
{
   HashNode **buckets;
   size_t     n_buckets;

   n_buckets = (Hash)((float)(hmap->n_items * 2) / (hmap->min_load + hmap->max_load));
   n_buckets = roundup_pow2(n_buckets);
   if (n_buckets == hmap->n_buckets)
      return;

   buckets = (HashNode **)calloc((size_t)n_buckets, sizeof(HashNode *));

   while (hmap->n_buckets--) {
      HashNode *hnode = hmap->buckets[hmap->n_buckets];
      while (hnode) {
         HashNode *next = hnode->next;
         Hash      hash = calc_hash(hnode->key, hnode->key_size, hmap->hash_fn, n_buckets);
         hnode->next = buckets[hash];
         buckets[hash] = hnode;
         hnode = next;
      }
   }

   free(hmap->buckets);
   hmap->n_buckets = n_buckets;
   hmap->buckets = buckets;
}

INLINE static HashNode *hashmap_find(HashMap *hmap, const void *key, size_t key_size, Hash hash)
{
   HashNode *hnode;

   for (hnode = hmap->buckets[hash]; hnode; hnode = hnode->next) {
      if (hashnode_eq(hnode, key, key_size))
         break;
   }

   return hnode;
}

void hashmap_init_with(HashMap *hmap, size_t key_size, HashFn hash_fn, Hash n_buckets)
{
   hmap->n_buckets = roundup_pow2(n_buckets);
   hmap->buckets = (HashNode **)calloc((size_t)hmap->n_buckets, sizeof(HashNode *));
   hmap->n_items = 0;
   hmap->key_size = key_size;
   hmap->hash_fn = hash_fn ? hash_fn : default_hash_fn;
   hmap->min_load = 0.25;
   hmap->max_load = 0.75;
}

void hashmap_init(HashMap *hmap, size_t key_size, HashFn hash_fn)
{
   hashmap_init_with(hmap, key_size, hash_fn, INITIAL_N_BUCKETS);
}

void hashmap_insert(HashMap *hmap, const void *key, void *value)
{
   HashEntry hentry;

   hashentry_init(&hentry, hmap, key);
   hashentry_set(&hentry, value, NULL);
}

bool hashmap_remove(HashMap *hmap, const void *key, void **pvalue)
{
   size_t    key_size = hashmap_key_size(hmap, key);
   Hash      hash = calc_hash(key, key_size, hmap->hash_fn, hmap->n_buckets);
   HashNode *hnode, *prev = NULL;

   for (hnode = hmap->buckets[hash]; hnode; hnode = hnode->next) {
      if (hashnode_eq(hnode, key, key_size))
         break;
      prev = hnode;
   }

   if (!hnode)
      return false;

   if (prev)
      prev->next = hnode->next;
   else
      hmap->buckets[hash] = hnode->next;
   hmap->n_items--;

   if (pvalue)
      *pvalue = hnode->value;
   free(hnode);

   return true;
}

bool hashmap_get(HashMap *hmap, const void *key, void **pvalue)
{
   HashEntry hentry;

   if (!hashentry_init(&hentry, hmap, key))
      return false;

   hashentry_value(&hentry, pvalue);

   return true;
}

bool hashmap_contains(const HashMap *hmap, const void *key)
{
   HashEntry hentry;
   return hashentry_init(&hentry, (HashMap *)hmap, key);
}

void hashmap_clear(HashMap *hmap)
{
   size_t i;

   for (i = 0; i < hmap->n_buckets; i++) {
      if (hmap->buckets[i]) {
         free(hmap->buckets[i]);
         hmap->buckets[i] = NULL;
      }
   }
   hmap->n_items = 0;
}

void hashmap_free(HashMap *hmap)
{
   while (hmap->n_buckets--) {
      if (hmap->buckets[hmap->n_buckets])
         free(hmap->buckets[hmap->n_buckets]);
   }
   free(hmap->buckets);
   memset(hmap, 0, sizeof(*hmap));
}

void hashmap_rehash(HashMap *hmap)
{
   float load_factor = hashmap_load_factor(hmap);
   if (load_factor < hmap->min_load || load_factor > hmap->max_load)
      hashmap_rehash_inner(hmap);
}

bool hashmap_merge(HashMap *dst, HashMap *src)
{
   HashIter hiter;

   if (dst->key_size != src->key_size)
      return false;

   hashiter_init(src, &hiter);
   while (hashiter_next(&hiter)) {
      hashmap_insert(dst, hashiter_key(&hiter), hashiter_value(&hiter));
   }
   hashmap_free(src);

   return true;
}

bool hashmap_set_thresholds(HashMap *hmap, float min_load, float max_load)
{
   if (min_load <= 0. || max_load <= 0. || min_load > max_load)
      return false;

   hmap->min_load = min_load;
   hmap->max_load = max_load;

   return true;
}

//
// MARK: HashEntry
//
bool hashentry_init(HashEntry *hentry, HashMap *hmap, const void *key)
{
   size_t key_size = hashmap_key_size(hmap, key);
   Hash   hash = calc_hash(key, key_size, hmap->hash_fn, hmap->n_buckets);

   hentry->hmap = hmap;
   hentry->hnode = hashmap_find(hmap, key, key_size, hash);
   hentry->key = key;
   hentry->key_size = key_size;
   hentry->hash = hash;

   return hentry->hnode != NULL;
}

bool hashentry_set(HashEntry *hentry, void *value, void **pvalue)
{
   HashMap  *hmap = hentry->hmap;
   HashNode *hnode = hentry->hnode;
   bool      found;

   if (!hnode) {
      found = false;
      if (pvalue)
         *pvalue = NULL;

      hnode = hashnode_new(hentry->key, hentry->key_size, value);
      hnode->next = hmap->buckets[hentry->hash];
      hmap->buckets[hentry->hash] = hnode;
      hmap->n_items++;

      hashmap_rehash(hmap);
   }
   else {
      found = true;
      if (pvalue)
         *pvalue = hnode->value;
      hnode->value = value;
   }

   hentry->hnode = hnode;

   return found;
}

//
// MARK: HashIter
//
void hashiter_init(const HashMap *hmap, HashIter *hiter)
{
   hiter->hmap = hmap;
   hiter->hnode = NULL;
   hiter->hash = (Hash)-1;
}

bool hashiter_next(HashIter *hiter)
{
   const HashMap *hmap = hiter->hmap;

   if (hiter->hnode)
      hiter->hnode = hiter->hnode->next;

   while (!hiter->hnode) {
      if (++hiter->hash >= hmap->n_buckets)
         return false;

      hiter->hnode = hmap->buckets[hiter->hash];
   }

   return true;
}

/**
 * TODO:
 * - optimize hashmap_merge
 * - is free_fn necessary?
 * - is hashmap_clear necessary?
 * - better hashiter_init?
 */