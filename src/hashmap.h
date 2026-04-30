/*
 * Copyright (c) 2026 Alessandro Martone
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**
 * @file hashmap.h
 */
#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#ifdef _MSC_VER
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

#define HMAP_KEY_SIZE_STR ((size_t)-1) /**< marker for keys that are variable length c-strings */

typedef uint32_t Hash; /**< type of the hash */
typedef Hash (*HashFn)(const void *key, size_t size); /**< custom hash function pointer */

typedef struct HashNode {
   const void      *key;
   size_t           key_size; /**< real key size, relevant when base_key_size is HMAP_KEY_STR */
   void            *val;
   struct HashNode *next;
} HashNode;

/**
 * @brief linked list-based hashmap
 * 
 * besides the nice HashEntry thing, this is kept pretty simple, with almost no "configurations".
 */
typedef struct HashMap {
   HashNode **buckets;
   Hash       n_buckets;
   size_t     n_items;
   size_t     base_key_size; /**< size of the keys if its fixed */
   HashFn     hash_fn;
} HashMap;

/**
 * @brief cache of a lookup inside the hashmap
 * 
 * allows to do multiple operations on the same key without having to look it up multiple times
 * 
 * @note any modification done to the hashmap, besides the ones done through this, invalidate this struct
 */
typedef struct HashEntry {
   HashMap    *map;
   HashNode   *node;
   HashNode   *prev;
   const void *key;
   size_t      key_size;
   Hash        hash;
} HashEntry;

/**
 * @brief state of a sequential iteration in the hashmap
 * 
 * iterations doesn't align with insertion order
 * 
 * @note any modification done to the hashmap, besides the ones done through this, invalidate this struct
 */
typedef struct HashIter {
   const HashMap  *map;
   const HashNode *node;
   Hash            hash;
} HashIter;

/**
 * @brief get entry corresponding to @p key
 * 
 * allows to do a handful of logic on one key without calculating the hash and finding the right node every time
 * 
 * @param[out] entry entry
 * @param[in] map hashmap
 * @param[in] key key to find
 * 
 * @return if @p key was found
 */
bool hashentry_init(HashEntry *entry, HashMap *map, const void *key);

/**
 * @brief update/insert the value of the entry
 * 
 * @param[in,out] entry
 * @param[in] val new value
 * @param[out] pval if != NULL, on success it points to the previous value
 * 
 * @return if the entry was occupied
 */
bool hashentry_set(HashEntry *entry, void *val, void **pval);

/**
 * @brief remove key+value pair from the hashmap
 * 
 * @param[in,out] entry entry
 * @param[out] pval on success, it points to the previous value
 * 
 * @return if @p key was found
 */
bool hashentry_remove(HashEntry *entry, void **pval);

/**
 * @brief key corresponding to the entry
 * 
 * @param[in] entry entry
 * @param[out] pkey on success, it points to the key
 * 
 * @return if the entry is occupied
 */
INLINE static bool hashentry_key(const HashEntry *entry, const void **pkey)
{
   if (!entry->node)
      return false;

   *pkey = entry->node->key;
   return true;
}

/**
 * @brief value corresponding to the entry
 * 
 * @param[in] entry entry
 * @param[out] pval on success, it points to the value
 * 
 * @return if the entry is occupied
 */
INLINE static bool hashentry_val(const HashEntry *entry, void **pval)
{
   if (!entry->node)
      return false;

   *pval = entry->node->val;
   return true;
}

#define hashentry_found(entry) ((entry)->node != NULL) /**< if entry is found */

/**
 * @brief initialize hashmap specifiying the initial number of buckets
 * @note this will allocate some buckets right away
 *       that's to avoid checking for NULL at every lookup and to avoid rehashing often on early insertions
 * 
 * @param[out] map hashmap
 * @param[in] base_key_size size of the keys. if they are variable c-strings, pass HMAP_KEY_STR
 * @param[in] hash_fn if != NULL, custom hash function
 */
void hashmap_init(HashMap *map, size_t base_key_size, HashFn hash_fn);

/**
 * @brief get value corresponding to key
 * 
 * @param[in] map hashmap
 * @param[in] key key to find
 * @param[out] pval_size if != NULL, it's set to the length of value. useful if HASHMAP_LEN_STR is used
 * 
 * @return pointer to the value, or NULL
 */
INLINE static bool hashmap_get(HashMap *map, const void *key, void **pval)
{
   HashEntry entry;

   if (!hashentry_init(&entry, map, key))
      return false;
      
   hashentry_val(&entry, pval);
   return true;
}

/**
 * @brief update value if the key exists, insert otherwise
 * 
 * @param[in,out] map
 * @param[in] key key to find/set
 * @param[in] val value to set
 * 
 * @return if @p key existed
 */
INLINE static bool hashmap_set(HashMap *map, const void *key, void *val, void **pval)
{
   HashEntry entry;
   hashentry_init(&entry, map, key);
   return hashentry_set(&entry, val, pval);
}

/**
 * @brief remove key+value pair from the hashmap
 * 
 * @param[in,out] map hashmap
 * @param[in] key key to remove
 * 
 * @return if @p key was found
 */
INLINE static bool hashmap_remove(HashMap *map, const void *key, void **pval)
{
   HashEntry entry;
   hashentry_init(&entry, map, key);
   return hashentry_remove(&entry, pval);
}

/**
 * @brief check if @p key exists in the hashmap
 */
INLINE static bool hashmap_contains(const HashMap *map, const void *key)
{
   HashEntry entry;
   return hashentry_init(&entry, (HashMap *)map, key);
}

/**
 * @brief free the hashmap
 * 
 * @param[in,out] map hashmap
 */
void hashmap_free(HashMap *map);

/**
 * @brief manually ask for a rehash
 * 
 * this still checks the thresholds. useful to implement rehashing after removal
 * 
 * @param[in,out] map hashmap
 */
void hashmap_rehash(HashMap *map);

/**
 * @brief number of elements in the hashmap
 */
INLINE static size_t hashmap_len(const HashMap *hmap)
{
   return hmap->n_items;
}

/**
 * @brief initialize iterator
 * 
 * @param[out] iter
 * @param[in] map
 */
INLINE static void hashiter_init(HashIter *iter, const HashMap *map)
{
   iter->map = map;
   iter->node = NULL;
   iter->hash = (Hash)-1;
}

/**
 * @brief get next element of the hashmap
 * 
 * @param[in,out] iter
 * 
 * @return if the was a next element
 */
bool hashiter_next(HashIter *iter);

/**
 * @brief return pointer to the corresponding key
 * @note valid only after a successful hashiter_next
 */
INLINE static const void *hashiter_key(const HashIter *iter)
{
   assert(iter->node);
   return iter->node->key;
}

/**
 * @brief return pointer to the corresponding value
 * @note valid only after a successful hashiter_next
 */
INLINE static const void *hashiter_val(const HashIter *iter)
{
   assert(iter->node);
   return (void *)iter->node->val;
}

#endif /* __HASHMAP_H__ */