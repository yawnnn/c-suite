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

#ifdef _MSC_VER
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

#define KEY_SIZE_STR ((size_t)-1) /**< marker for keys that are variable length c-strings */

typedef uint32_t Hash; /**< type of the hash */
typedef Hash (*HashFn)(const void *key, size_t size); /**< custom hash function pointer */

typedef struct HashNode {
   const void      *key;
   size_t           key_size; /**< needed when the hashmap's key_size is KEY_SIZE_STR */
   void            *value;
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
   size_t     key_size;
   HashFn     hash_fn;
   float      min_load;
   float      max_load;
} HashMap;

/**
 * @brief cache of a lookup inside the hashmap
 * 
 * allows to do multiple operations on the same key without having to look it up multiple times
 * 
 * @note any modification done to the hashmap, besides the ones done through this, invalidate this struct
 */
typedef struct HashEntry {
   HashMap    *hmap;
   HashNode   *hnode;
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
   const HashMap  *hmap;
   const HashNode *hnode;
   Hash            hash;
} HashIter;

/**
 * @brief initialize hashmap specifiying the initial number of buckets
 * 
 * @param[out] hmap hashmap
 * @param[in] key_size size of the keys. if they are variable c-strings, pass KEY_SIZE_STR
 * @param[in] hash_fn if != NULL, custom hash function
 * @param[in] n_buckets minimum number of initial buckets
 */
void hashmap_init_with(HashMap *hmap, size_t key_size, HashFn hash_fn, Hash n_buckets);

/**
 * @brief initialize hashmap
 * 
 * see @p hashmap_init_with for params
 */
void hashmap_init(HashMap *hmap, size_t key_size, HashFn hash_fn);

/**
 * @brief insert key+value pair in the hashmap
 * 
 * @param[in,out] hmap hashmap
 * @param[in] key key to insert
 * @param[in] value value to insert
 */
void hashmap_insert(HashMap *hmap, const void *key, void *value);

/**
 * @brief remove key+value pair from the hashmap
 * 
 * @param[in,out] hmap hashmap
 * @param[in] key key to remove
 * @param[out] pvalue if != NULL, on success it points to the removed value
 * 
 * @return if @p key was found
 */
bool hashmap_remove(HashMap *hmap, const void *key, void **pvalue);

/**
 * @brief get value corresponding to key
 * 
 * @param[in] hmap hashmap
 * @param[in] key key to find
 * @param[out] pvalue on success, contains the pointer to the value
 * 
 * @p pvalue is not set on failure, so that can be used as a default value by the caller
 * 
 * @return if @p key was found
 */
bool hashmap_get(HashMap *hmap, const void *key, void **pvalue);

/**
 * @brief check if @p key exists in the hashmap
 * 
 * @param[in] hmap hashmap
 * @param[in] key key to find
 * 
 * @return if @p key was found
 */
bool hashmap_contains(const HashMap *hmap, const void *key);

/**
 * @brief remove all elements from the hashmap, but don't free underlying buckets
 * 
 * @param[in,out] hmap hashmap
 */
void hashmap_clear(HashMap *hmap);

/**
 * @brief free the hashmap
 * 
 * @param[in,out] hmap hashmap
 */
void hashmap_free(HashMap *hmap);

/**
 * @brief manually ask for a rehash
 * 
 * this still checks the thresholds. useful to implement rehashing after removal
 * 
 * @param[in,out] hmap hashmap
 */
void hashmap_rehash(HashMap *hmap);

/**
 * @brief merge one hashmap into the other
 * 
 * @param[in,out] dst destination hashmap
 * @param[in,out] src source hashmap. it will be consumed on exit
 * 
 * @return if key sizes are the same, otherwise maps aren't compatible and no work was done
 */
bool hashmap_merge(HashMap *dst, HashMap *src);

/**
 * @brief set thresholds for rehashing
 * 
 * @param[in,out] hmap hashmap
 * @param[in] min_load minimum load. affects only manual rehashing
 * @param[in] max_load maximum load
 * 
 * default is @p min_load = 0.25 and @p max_load = 0.75
 * 
 * @return if the values are valid
 */
bool hashmap_set_thresholds(HashMap *hmap, float min_load, float max_load);

/**
 * @brief number of elements in the hashmap
 */
INLINE static size_t hashmap_len(const HashMap *hmap)
{
   return hmap->n_items;
}

/**
 * @brief get entry corresponding to @p key
 * 
 * allows to do a handful of logic on one key without calculating the hash and finding the right node every time
 * 
 * @param[out] hentry entry
 * @param[in] hmap hashmap
 * @param[in] key key to find
 * 
 * @return if @p key was found
 */
bool hashentry_init(HashEntry *hentry, HashMap *hmap, const void *key);

/**
 * @brief update/insert the value of the entry
 * 
 * @param[in,out] hentry
 * @param[in] value new value
 * @param[out] pvalue if != NULL, on success it points to the previous value
 * 
 * @return if the entry was occupied
 */
bool hashentry_set(HashEntry *hentry, void *value, void **pvalue);

/**
 * @brief key corresponding to the entry
 * 
 * @param[in] hentry entry
 * @param[out] pkey on success, it points to the key
 * 
 * @return if the entry is occupied
 */
INLINE static bool hashentry_key(const HashEntry *hentry, const void **pkey)
{
   if (!hentry->hnode)
      return false;

   *pkey = hentry->hnode->key;

   return true;
}

/**
 * @brief value corresponding to the entry
 * 
 * @param[in] hentry entry
 * @param[out] pvalue on success, it points to the value
 * 
 * @return if the entry is occupied
 */
INLINE static bool hashentry_value(const HashEntry *hentry, void **pvalue)
{
   if (!hentry->hnode)
      return false;

   *pvalue = hentry->hnode->value;

   return true;
}

/**
 * @brief if the key was found on init
 */
INLINE static bool hashentry_is_occupied(const HashEntry *hentry)
{
   return hentry->hnode != NULL;
}

/**
 * @brief if the key was not found on init
 */
INLINE static bool hashentry_is_vacant(const HashEntry *hentry)
{
   return hentry->hnode == NULL;
}

/**
 * @brief initialize iterator
 * 
 * @param[in] hmap
 * @param[out] hiter
 */
void hashiter_init(const HashMap *hmap, HashIter *hiter);

/**
 * @brief get next element of the hashmap
 * 
 * @param[in,out] hiter
 * 
 * @return if the was a next element
 */
bool hashiter_next(HashIter *hiter);

/**
 * @brief return pointer to the corresponding key
 * 
 * @note valid only after a successful hashiter_next
 */
INLINE static const void *hashiter_key(const HashIter *hiter)
{
   return hiter->hnode->key;
}

/**
 * @brief return pointer to the corresponding value
 * 
 * @note valid only after a successful hashiter_next
 */
INLINE static void *hashiter_value(const HashIter *hiter)
{
   return (void *)hiter->hnode->value;
}

#endif /* __HASHMAP_H__ */