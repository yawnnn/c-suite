#include "hashmap.h"

static uint64_t make_hash(void *key) {
    return (uint64_t)key;
}

static inline void hashmap_free_entry(HashEntry *entry) {
    free(entry);
}

void hashmap_new(HashMap *hm) {
    memset(hm, 0, sizeof(HashMap));
}

void hashmap_free(HashMap *hm) {
    if (hm->buckets)
        free(hm->buckets);

    if (hm->items) {
        HashEntry *curr, *next;

        curr = hm->items;
        next = curr->next;

        while (next) {
            curr = next;
            next = next->next;
            hashmap_free_node(curr);
        }

        free(hm->items);
    }
}

static inline HashEntry *hashmap_get_bucket(HashMap *hm, uint64_t hash) {
    size_t pos;

    if (!hm->nbucket)
        return NULL;

    pos = (size_t)(hash % hm->nbucket);

    return hm->buckets[pos];
}

static HashEntry *hashmap_new_bucket(HashMap *hm) {
    HashEntry *bucket;

    if (hm->nbucket++)
        hm->buckets = realloc(hm->buckets, hm->nbucket * sizeof(HashEntry *));
    else
        hm->buckets = malloc(hm->nbucket * sizeof(HashEntry *));

    bucket = calloc(1, sizeof(HashEntry));
    hm->buckets[hm->nbucket - 1] = bucket;

    return bucket;
}

static HashEntry *hashmap_new_entry(HashMap *hm, HashEntry *bucket)
{
    HashEntry *entry;

    entry = calloc(1, sizeof(HashEntry));
    entry->next = bucket->next;
    bucket->next = entry;

    return entry;
}

void hashmap_insert(HashMap *hm, void *key, void *value) {
    uint64_t hash;
    HashEntry *bucket, *entry;

    hash = make_hash(key);
    bucket = hashmap_get_bucket(hm, hash);

    if (!bucket)
        bucket = hashmap_new_bucket(hm);

    entry = bucket->next;
    while (entry && hash != entry->node.hash)
        entry = entry->next;

    if (!entry) {
        entry = hashmap_new_entry(hm, bucket);
        entry->node.hash = hash;
        entry->node.key = key;
    }

    entry->node.value = value;
}

void hashmap_remove(HashMap *hm, void *key) {
    ;
}

void *hashmap_get(HashMap *hm, void *key) {
    ;
}