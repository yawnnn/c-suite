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

#define HASHMAP_LEN_STR ((size_t)-1) /**< marker for keys that are variable length c-strings */

#if defined(__STDC__) && __STDC_VERSION__ >= 201112L
   #include "stddef.h"
   #include "stdalign.h"
   #define MAX_ALIGNMENT alignof(max_align_t)
#else
   /**
    * @brief generic alignment
    * types like double and uint64_t can have 8byte alignment even on 32bit
    * alignof(max_align_t) can be 16 for things like long doubles and SIMD types, but i keep this more conservative
    */
   #define MAX_ALIGNMENT 8
#endif

/**< with MAX_ALIGNMENT == 8 this isn't necessary, as sizeof(HashNode) % 8 == 0 */
#define ALIGN_UP(num)      (((uintptr_t)(num) + MAX_ALIGNMENT - 1) & ~(MAX_ALIGNMENT - 1))
#define HASHNODE_KEY(node) ((void *)ALIGN_UP((node) + 1))

typedef uint32_t Hash; /**< type of the hash */
typedef Hash (*HashFn)(const void *key, size_t size); /**< custom hash function pointer */

typedef struct HashNode {
   struct HashNode *next;
   void            *val;
   uint32_t         val_size; /**< value size. on 64bit this is "free", as it would be padding otherwise */
   Hash             hash; /**< key's hash */
} HashNode;

/**
 * @brief linked list-based hashmap
 * 
 * the API is kept intentionally simple, as there's lots that one might want to configure which can just clutter the API when it's not needed
 * those toggles should be easy enough to add, if required.
 * see @p hashmap_init for more details
 * 
 * also check-out @p HashEntry below, as it allows for optimizations and more control
 */
typedef struct HashMap {
   HashNode **buckets; /**< array of buckets */
   Hash       n_buckets; /**< number of buckets */
   size_t     n_items; /**< item count */
   size_t     base_key_size; /**< size of the keys if its constant, or HASHMAP_LEN_STR */
   size_t     base_val_size; /**< size of the values if its constant, or HASHMAP_LEN_STR */
   HashFn     hash_fn; /**< hash function in use */
} HashMap;

/**
 * @brief entry in the hashmap
 * 
 * allows to do multiple operations on a key+value pair (or lack thereof) without having to look it up multiple times
 * 
 * @note modifications to the hashmap not done through this, can invalidate this
 */
typedef struct HashEntry {
   HashMap    *map;
   HashNode   *node; /**< node found/inserted */
   HashNode   *prev; /**< node before the one found */
   const void *key; /**< key found/inserted */
   size_t      key_size;
   Hash        hash;
} HashEntry;

/**
 * @brief sequential iterator over every key+value pair
 * 
 * iterations doesn't align with insertion order
 * 
 * @note modifications to the hashmap can invalidate this
 */
typedef struct HashIter {
   const HashMap  *map;
   const HashNode *node; /**< current node */
   Hash            idx; /**< current bucket index */
} HashIter;

/**
 * @brief lookup @p key and prepare @p entry struct
 * 
 * @param[out] entry entry
 * @param[in] map hashmap
 * @param[in] key key to find
 * 
 * @return if @p key was found
 */
bool hashentry_init(HashEntry *entry, HashMap *map, const void *key);

/**
 * @brief update value if the key exists, insert otherwise
 * 
 * @param[in,out] entry
 * @param[in] val value to set
 * @param[out] pval if != NULL, the previous value is not freed and this is the pointer to it
 * @param[out] pval_size if != NULL, it's set to the previous value's length. useful if HASHMAP_LEN_STR is used
 * 
 * @return if the key existed
 */
bool hashentry_set(HashEntry *entry, const void *val, void **pval, size_t *pval_size);

/**
 * @brief remove key+value pair from the hashmap
 * 
 * @param[in,out] entry entry
 * @param[out] pval if != NULL, the value is not freed and this is the pointer to it
 * @param[out] pval_size if != NULL, it's set to the value's length. useful if HASHMAP_LEN_STR is used
 * 
 * @return if @p key was found
 */
bool hashentry_remove(HashEntry *entry, void **pval, size_t *pval_size);

/**
 * @brief key corresponding to the entry
 * 
 * @param[in] entry entry
 * @param[out] pkey on success, it points to the key
 * 
 * @return pointer to the key, or NULL
 */
INLINE static const void *hashentry_key(const HashEntry *entry)
{
   if (!entry->node)
      return NULL;
   return HASHNODE_KEY(entry->node);
}

/**
 * @brief value corresponding to the entry
 * 
 * @param[in] entry entry
 * @param[out] pval_size if != NULL, on success is set to the size of the value. useful if HASHMAP_LEN_STR is used
 * 
 * @return pointer to the value, or NULL
 */
INLINE static const void *hashentry_val(const HashEntry *entry, size_t *pval_size)
{
   if (!entry->node)
      return NULL;

   if (pval_size)
      *pval_size = (size_t)entry->node->val_size;
   return entry->node->val;
}

#define hashentry_found(entry) ((entry)->node != NULL) /**< if entry is found */

/**
 * @brief initialize hashmap
 * 
 * @note value sizes above 2^32-1 are not supported and will silently truncate
 *       this is to make full use of the space occupied by @p HashNode
 * @note both keys and values are always cloned by the hashmap, for API simplicity
 * @note variable length keys/values that are not c-strings are not supported, for API simplicity
 * 
 * @param[out] map hashmap
 * @param[in] base_key_size size of the keys. if they are variable length c-strings, pass HASHMAP_LEN_STR
 * @param[in] base_val_size size of the values. if they are variable length c-strings, pass HASHMAP_LEN_STR
 * @param[in] hash_fn if != NULL, custom hash function
 */
void hashmap_new(HashMap *map, size_t base_key_size, size_t base_val_size, HashFn hash_fn);

/**
 * @brief get value corresponding to key
 * 
 * @param[in] map hashmap
 * @param[in] key key to find
 * @param[out] pval_size if != NULL, it's set to the length of value. useful if HASHMAP_LEN_STR is used
 * 
 * @return pointer to the value, or NULL
 */
INLINE static const void *hashmap_get(HashMap *map, const void *key, size_t *pval_size)
{
   HashEntry entry;
   hashentry_init(&entry, map, key);
   return hashentry_val(&entry, pval_size);
}

/**
 * @brief update value if the key exists, insert otherwise
 * 
 * @param[in,out] map
 * @param[in] key key to find/set
 * @param[in] val value to set
 * @param[out] pval if != NULL, the previous value is not freed and this is the pointer to it
 * @param[out] pval_size if != NULL, it's set to the previous value's length. useful if HASHMAP_LEN_STR is used
 * 
 * @return if @p key existed
 */
INLINE static bool hashmap_set(HashMap *map, const void *key, const void *val, void **pval, size_t *pval_size)
{
   HashEntry entry;
   hashentry_init(&entry, map, key);
   return hashentry_set(&entry, val, pval, pval_size);
}

/**
 * @brief remove key+value pair from the hashmap
 * 
 * @param[in,out] map hashmap
 * @param[in] key key to remove
 * @param[out] pval if != NULL, the value is not freed and this is the pointer to it
 * @param[out] pval_size if != NULL, it's set to the value's length. useful if HASHMAP_LEN_STR is used
 * 
 * @return if @p key was found
 */
INLINE static bool hashmap_remove(HashMap *map, const void *key, void **pval, size_t *pval_size)
{
   HashEntry entry;
   hashentry_init(&entry, map, key);
   return hashentry_remove(&entry, pval, pval_size);
}

/**
 * @brief check if @p key exists in the hashmap
 */
INLINE static bool hashmap_contains(HashMap *map, const void *key)
{
   HashEntry entry;
   return hashentry_init(&entry, map, key);
}

/**
 * @brief manually request a rehash
 * 
 * this still checks the thresholds.
 * right now this is done automatically on insert but not on removal, so that's when you might wanna use it
 * 
 * @param[in,out] map hashmap
 * 
 * @return if the rehash happened
 */
bool hashmap_rehash(HashMap *map);

/**
 * @brief free all the memory
 * 
 * @param[in,out] map hashmap
 */
void hashmap_free(HashMap *map);

/**
 * @brief number of key+value pairs in the hashmap
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
   iter->idx = (Hash)-1;
}

/**
 * @brief step on next element of the hashmap
 * 
 * @param[in,out] iter iterator
 * 
 * @return if the iterator is not exhausted
 */
bool hashiter_next(HashIter *iter);

/**
 * @brief pointer to the current key
 * @note valid only after a successful hashiter_next
 */
INLINE static const void *hashiter_key(const HashIter *iter)
{
   assert(iter->node);
   return HASHNODE_KEY(iter->node);
}

/**
 * @brief pointer to the current value
 * @note valid only after a successful hashiter_next
 */
INLINE static const void *hashiter_val(const HashIter *iter, size_t *pval_size)
{
   assert(iter->node);
   if (pval_size)
      *pval_size = (size_t)iter->node->val_size;
   return iter->node->val;
}

#endif /* __HASHMAP_H__ */