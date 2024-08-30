/**
 * @file hashmap.h
 */

#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct HashNode {
    uint64_t hash;
    void *key;
    void *value;
} HashNode;

typedef struct HashEntry {
    HashNode node;
    struct HashEntry *next;
} HashEntry;

/**
 * @brief hashmap
 */
typedef struct HashMap {
    HashEntry *items;
    size_t nitem;
    HashEntry **buckets;
    size_t nbucket;
} HashMap;

void hashmap_new(HashMap *h);
void hashmap_free(HashMap *hm);
void *hashmap_get(HashMap *hm, void *key);
void hashmap_insert(HashMap *hm, void *key, void *value);
void hashmap_remove(HashMap *hm, void *key);

#endif /* __HASHMAP_H__ */