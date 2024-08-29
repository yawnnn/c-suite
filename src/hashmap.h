/**
 * @file hashmap.h
 */

#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct HashValue {
    uint64_t hash;
    void *key;
    void *value;
} HashValue;

typedef struct HashNode {
    HashValue value;
    struct HashNode *next;
} HashNode;

/**
 * @brief hashmap
 */
typedef struct HashMap {
    HashNode *items;
    size_t nitem;
    HashNode **buckets;
    size_t nbucket;
} HashMap;

void hashmap_new(HashMap *h);
void hashmap_free(HashMap *hm);
void *hashmap_get(HashMap *hm, void *key);
void hashmap_insert(HashMap *hm, void *key, void *value);
void hashmap_remove(HashMap *hm, void *key);

#endif /* __HASHMAP_H__ */