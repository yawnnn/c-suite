#include "hashmap.h"

static uint64_t make_hash(void *key) {
    return (uint64_t)key;
}

static void hashmap_free_node(HashNode *hmnode) {
    free(hmnode);
}

void hashmap_new(HashMap *hm) {
    memset(hm, 0, sizeof(HashMap));
}

void hashmap_free(HashMap *hm) {
    if (hm->buckets)
        free(hm->buckets);

    if (hm->items) {
        HashNode *curr, *next;

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

static inline HashNode *hashmap_get_bucket(HashMap *hm, uint64_t hash) {
    size_t pos;

    if (!hm->nbucket)
        return NULL;

    pos = (size_t)(hash % hm->nbucket);

    return hm->buckets[pos];
}

static HashNode *hashmap_new_bucket(HashMap *hm) {
    HashNode **pbucket;

    if (hm->nbucket++)
        hm->buckets = realloc(hm->buckets, hm->nbucket * sizeof(HashNode *));
    else
        hm->buckets = malloc(hm->nbucket * sizeof(HashNode *));

    pbucket = &hm->buckets[hm->nbucket - 1];
    *pbucket = calloc(1, sizeof(HashNode));

    return *pbucket;
}

static HashNode *hashmap_new_node(HashMap *hm, HashNode *bucket)
{
    HashNode *node;

    node = calloc(1, sizeof(HashNode));
    node->next = bucket->next;
    bucket->next = node;

    return node;
}

void hashmap_insert(HashMap *hm, void *key, void *value) {
    uint64_t hash;
    HashNode *bucket, *node;

    hash = make_hash(key);
    bucket = hashmap_get_bucket(hm, hash);

    if (!bucket)
        bucket = hashmap_new_bucket(hm);

    node = bucket->next;
    while (node && hash != node->value.hash)
        node = node->next;

    if (!node) {
        node = hashmap_new_node(hm, bucket);
        node->value.hash = hash;
        node->value.key = key;
    }

    node->value.value = value;
}

void hashmap_remove(HashMap *hm, void *key) {
    ;
}

void *hashmap_get(HashMap *hm, void *key) {
    ;
}