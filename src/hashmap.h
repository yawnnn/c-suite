/**
 * @file hashmap.h
 */

#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct HashNode {
   uint64_t hash;
   void    *key;
   void    *value;
} HashNode;

typedef struct HashBucket {
   HashNode *ptr;
   size_t    len;
   size_t    cap;
} HashBucket;

typedef uint32_t hash_t;

typedef hash_t (*hashfunc_t)(const void *key, size_t size);

typedef struct HashMap {
   HashBucket *buckets;
   size_t      nbucket;
   size_t      nitem;

   size_t key_size;
   size_t value_size;

   hashfunc_t hash_func;
} HashMap;

void hashmap_new(HashMap *hm, size_t key_size, size_t value_size);
void hashmap_free(HashMap *hm);
bool hashmap_insert(HashMap *hm, void *key, void *value, void *prev);
bool hashmap_remove(HashMap *hm, void *key, void *prev);
bool hashmap_get(HashMap *hm, void *key, void *value);

#endif /* __HASHMAP_H__ */