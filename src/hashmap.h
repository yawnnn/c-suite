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
    void *key;
    void *value;
} HashNode;

typedef struct HashEntry {
    HashNode node;
    struct HashEntry *next;
} HashEntry;

typedef struct HashBucket {
    struct HashEntry *head;
} HashBucket;

typedef struct HashMap {
    HashBucket *buckets;
    size_t nbucket;
    size_t nitem;
} HashMap;

void hashmap_new(HashMap *hm);
void hashmap_free(HashMap *hm);
void *hashmap_insert(HashMap *hm, void *key, void *value);
void *hashmap_remove(HashMap *hm, void *key);
void *hashmap_get(HashMap *hm, void *key);

#endif /* __HASHMAP_H__ */