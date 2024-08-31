#include "hashmap.h"

// Perl's hash function
static hash_t hash_func_generic(const void *key, size_t size) {
    register const uint8_t *data;
    register size_t i;
    register hash_t hash;

    const hash_t seed = 0;

    data = (const uint8_t *)key;
    i = size;
    hash = seed;

    while (i--) {
        hash += *data++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

static inline void hashmap_free_entry(HashEntry *entry) {
    free(entry);
}

static void hashmap_free_entries(HashEntry *head) {
    HashEntry *curr, *next;

    if (!head) return;

    curr = head;
    next = curr->next;

    while (next) {
        curr = next;
        next = next->next;
        hashmap_free_entry(curr);
    }

    hashmap_free_entry(head);
}

static void hashmap_grow(HashMap *hm) {
    if (hm->nbucket) {
        size_t old_nbucket = hm->nbucket;

        hm->nbucket = hm->nbucket * 2;
        hm->buckets = realloc(hm->buckets, hm->nbucket * sizeof(HashBucket));
        memset(
            hm->buckets + old_nbucket,
            0,
            (hm->nbucket - old_nbucket) * sizeof(HashBucket)
        );
    } else {
        hm->nbucket = 8;
        hm->buckets = calloc(hm->nbucket, sizeof(HashBucket));
    }
}

static inline HashBucket *hashmap_get_bucket(HashMap *hm, hash_t hash) {
    size_t pos;

    if (!hm->nbucket) return NULL;

    pos = (size_t)(hash % hm->nbucket);

    return &hm->buckets[pos];
}

static HashEntry *hashmap_new_entry(HashMap *hm, HashBucket *bucket) {
    HashEntry *entry;

    entry = calloc(1, sizeof(HashEntry));
    entry->next = bucket->head;
    bucket->head = entry;

    return entry;
}

static void *hashmap_remove_entry(HashBucket *bucket, hash_t hash) {
    HashEntry *curr, *prev;
    void *value = NULL;

    prev = NULL;
    curr = bucket->head;
    while (curr && hash != curr->node.hash) {
        prev = curr;
        curr = curr->next;
    }

    if (curr) {
        if (prev)
            prev->next = curr->next;
        else
            bucket->head = curr->next;

        value = curr->node.value;
        hashmap_free_entry(curr);
    }

    return value;
}

static HashEntry *hashmap_find_entry(HashBucket *bucket, hash_t hash) {
    HashEntry *entry;

    if (!bucket) return NULL;

    entry = bucket->head;
    while (entry && hash != entry->node.hash)
        entry = entry->next;

    return entry;
}

void hashmap_new(HashMap *hm, size_t key_size, size_t value_size) {
    hm->buckets = NULL;
    hm->nbucket = 0;
    hm->nitem = 0;

    hm->key_size = key_size;

    hm->hash_func = hash_func_generic;
}

void hashmap_free(HashMap *hm) {
    while (hm->nbucket--)
        hashmap_free_entries(hm->buckets[hm->nbucket].head);

    if (hm->buckets) free(hm->buckets);

    memset(hm, 0, sizeof(HashMap));
}

void *hashmap_insert(HashMap *hm, void *key, void *value) {
    HashBucket *bucket;
    HashEntry *entry;
    hash_t hash;
    void *old_value = NULL;

    hash = hm->hash_func(key, hm->key_size);

    bucket = hashmap_get_bucket(hm, hash);
    if (!bucket) {
        hashmap_grow(hm);
        bucket = hashmap_get_bucket(hm, hash);
    }

    entry = hashmap_find_entry(bucket, hash);

    if (entry)
        old_value = entry->node.value;
    else {
        entry = hashmap_new_entry(hm, bucket);
        entry->node.hash = hash;
        entry->node.key = key;
        hm->nitem++;
    }

    entry->node.value = value;

    return old_value;
}

void *hashmap_remove(HashMap *hm, void *key) {
    HashBucket *bucket;
    hash_t hash;
    void *value;

    hash = hm->hash_func(key, hm->key_size);

    bucket = hashmap_get_bucket(hm, hash);
    if (!bucket) return NULL;

    value = hashmap_remove_entry(bucket, hash);
    if (value) hm->nitem--;

    return value;
}

void *hashmap_get(HashMap *hm, void *key) {
    HashBucket *bucket;
    HashEntry *entry;
    hash_t hash;

    hash = hm->hash_func(key, hm->key_size);

    bucket = hashmap_get_bucket(hm, hash);
    if (!bucket) return NULL;

    entry = hashmap_find_entry(bucket, hash);

    return entry ? entry->node.value : NULL;
}