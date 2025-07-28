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

#define OBJ_SIZE_STR      ((size_t)-1) /**< marker for keys that are variable length c-strings */

#define FLG_REHASH_ON_INS 0x01 /**< check thresholds and rehash if necessary on insertion */
#define FLG_REHASH_ON_REM 0x02 /**< check thresholds and rehash if necessary on removal */

typedef uint32_t Hash; /**< type of the hash */
typedef Hash (*HashFn)(const void *key, size_t size); /**< custom hash function pointer */
typedef void (*FreeFn)(void *ptr); /**< custom free function pointer */

typedef struct HashNode {
   const void      *key;
   size_t           key_size; /**< needed when the hashmap's key_size is OBJ_SIZE_STR */
   void            *value;
   struct HashNode *next;
} HashNode;

/**
 * @brief HashMap implementation
 */
typedef struct HashMap {
   HashNode **buckets;
   Hash       n_buckets;
   size_t     n_items;
   size_t     key_size;
   HashFn     hash_fn;
   FreeFn     free_fn;
   float      min_load;
   float      max_load;
   uint8_t    flags;
} HashMap;

/**
 * @brief cache of a lookup inside the hashmap
 * 
 * allows to do a handful of logic on one key without calculating the hash and finding the right node every time
 * 
 * WARNING: any modification done to the hashmap, besides the ones done through this, invalidate this struct
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
 * WARNING: any modification done to the hashmap, besides the ones done through this, invalidate this struct
 */
typedef struct HashIter {
   HashMap  *hmap;
   HashNode *hnode;
   Hash      hash;
} HashIter;

/**
 * @brief initialize hashmap specifiying also the initial number of buckets
 * 
 * @param[out] hmap hashmap
 * @param[in] key_size size of the keys. if they are variable c-strings, pass OBJ_SIZE_STR
 * @param[in] hash_fn if != NULL, custom hash function
 * @param[in] free_fn if == NULL, values are never freed by the hashmap. if != NULL, values are always freed through this function
 * @param[in] n_buckets minimum number of initial buckets
 */
void hashmap_init_with(
   HashMap *hmap,
   size_t   key_size,
   HashFn   hash_fn,
   FreeFn   free_fn,
   Hash     n_buckets
);

/**
 * @brief initialize hashmap
 * 
 * see @p hashmap_init_with for params
 */
void hashmap_init(HashMap *hmap, size_t key_size, HashFn hash_fn, FreeFn free_fn);

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
 * @param[out] pvalue on success, if != NULL and free_fn is not specified, contains the pointer to the value
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
bool hashmap_contains(HashMap *hmap, const void *key);

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
 * @param[in,out] hmap hashmap
 * @param[in] n_buckets if != 0, the number of buckets to rehash with
 */
void hashmap_rehash(HashMap *hmap, Hash n_buckets);

/**
 * @brief merge one hashmap into the other
 * 
 * @param[in,out] dst destination hashmap
 * @param[in,out] src source hashmap. it will be consumed on exit
 */
void hashmap_merge(HashMap *dst, HashMap *src);

/**
 * @brief set thresholds for automatic rehashing
 * 
 * @param[in,out] hmap hashmap
 * @param[in] min_load if FLG_REHASH_ON_REM is set. minimum load below which a rehashing is triggered
 * @param[in] max_load if FLG_REHASH_ON_INS is set. maximum load below which a rehashing is triggered
 * 
 * default is @p min_load = 0.25 and @p max_load = 0.75
 */
void hashmap_set_thresholds(HashMap *hmap, float min_load, float max_load, bool rehash);

/**
 * @brief set flags
 * 
 * eg. hashmap_set_flags(hmap, FLG_REHASH_ON_INS | FLG_REHASH_ON_REM);
 * 
 * @param[out] hmap hashmap
 * @param[in] flags bitwise or (|) of the FLG_? desired
 */
INLINE static void hashmap_set_flags(HashMap *hmap, uint8_t flags)
{
   hmap->flags = flags;
}

/**
 * @brief if hashmap is empty
 */
INLINE static bool hashmap_is_empty(HashMap *hmap)
{
   return hmap->n_items == 0;
}

/**
 * @brief number of elements in the hashmap
 */
INLINE static size_t hashmap_len(HashMap *hmap)
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
 */
void hashentry_set(HashEntry *hentry, void *value);

/**
 * @brief key corresponding to the entry
 * 
 * @param[in] hentry entry
 * @param[out] pkey on success, points to the key
 * 
 * @return if the key was found on init
 */
INLINE static bool hashentry_key(HashEntry *hentry, const void **pkey)
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
 * @param[out] pkey on success, points to the value
 * 
 * @return if the key was found on init
 */
INLINE static bool hashentry_value(HashEntry *hentry, void **pvalue)
{
   if (!hentry->hnode)
      return false;

   *pvalue = hentry->hnode->value;

   return true;
}

/**
 * @brief if the key was found on init
 */
INLINE static bool hashentry_is_occupied(HashEntry *hentry)
{
   return hentry->hnode != NULL;
}

/**
 * @brief if the key was not found on init
 */
INLINE static bool hashentry_is_vacant(HashEntry *hentry)
{
   return hentry->hnode == NULL;
}

/**
 * @brief initialize iterator
 * 
 * @param[in] hmap
 * @param[out] hiter
 */
void hashiter_init(HashMap *hmap, HashIter *hiter);

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
 * WARNING: valid only after a successful hashiter_next
 */
INLINE static const void *hashiter_key(HashIter *hiter)
{
   return hiter->hnode->key;
}

/**
 * @brief return pointer to the corresponding value
 * 
 * WARNING: valid only after a successful hashiter_next
 */
INLINE static void *hashiter_value(HashIter *hiter)
{
   return hiter->hnode->value;
}

#endif /* __HASHMAP_H__ */