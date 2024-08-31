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

    return hash;  // can this be 0?
}

static void hashbucket_free(HashBucket *bucket) {
    free(bucket->ptr);
    bucket->cap = bucket->len = 0;
}

static void hashbucket_grow(HashBucket *bucket) {
    if (bucket->cap) {
        bucket->cap = bucket->cap * 2;
        bucket->ptr = realloc(bucket->ptr, bucket->cap * sizeof(HashNode));
    } else {
        bucket->cap = 1;
        bucket->ptr = malloc(bucket->cap, sizeof(HashNode));
    }
}

static void *
hashbucket_insert(HashBucket *bucket, hash_t hash, void *key, void *value) {
    HashNode *node;
    void *prev = NULL;

    node = hashbucket_find(bucket, hash);
    if (node)
        prev = node->value;
    else {
        hashbucket_grow(bucket);
        node = &bucket->ptr[bucket->len];
        node->hash = hash;
        node->key = key;
    }

    node->value = value;

    return prev;
}

static void *hashbucket_remove(HashBucket *bucket, hash_t hash) {
    HashNode *node;
    void *prev = NULL;

    node = hashbucket_find(bucket, hash);
    if (node) {
        prev = node->value;
        node->hash = 0;
    }

    return prev;
}

static HashNode *hashbucket_find(HashBucket *bucket, hash_t hash) {
    for (size_t i = 0; i < bucket->len; i++) {
        if (bucket->ptr[i].hash == hash) {
            return &bucket->ptr[i];
        }
    }

    return NULL;
}

static void hashmap_grow(HashMap *hm) {
    // nbucket is always kept at a power of 2
    if (hm->nbucket) {
        size_t old_nbucket = hm->nbucket;

        hm->nbucket = hm->nbucket * 2;
        hm->buckets = realloc(hm->buckets, hm->nbucket * sizeof(HashBucket));
        memset(&hm->buckets[old_nbucket], 0, old_nbucket * sizeof(HashBucket));
    } else {
        hm->nbucket = 8;
        hm->buckets = calloc(hm->nbucket, sizeof(HashBucket));
    }
}

static inline HashBucket *hashmap_get_bucket(HashMap *hm, hash_t hash) {
    size_t pos;

    if (!hm->nbucket) return NULL;

    // should be faster than modulo
    // possible because nbucket is always a power of 2
    pos = hash & (hm->nbucket - 1);

    return &hm->buckets[pos];
}

void hashmap_new(HashMap *hm, size_t key_size, size_t value_size) {
    hm->buckets = NULL;
    hm->nbucket = hm->nitem = 0;

    hm->key_size = key_size;

    hm->hash_func = hash_func_generic;
}

void hashmap_free(HashMap *hm) {
    while (hm->nbucket--)
        hashbucket_free(&hm->buckets[hm->nbucket]);
    free(hm->buckets);
    memset(hm, 0, sizeof(HashMap));
}

void *hashmap_insert(HashMap *hm, void *key, void *value) {
    HashBucket *bucket;
    HashNode *node;
    hash_t hash;
    void *old_value = NULL;

    hash = hm->hash_func(key, hm->key_size);

    bucket = hashmap_get_bucket(hm, hash);
    if (!bucket) {
        hashmap_grow(hm);
        bucket = hashmap_get_bucket(hm, hash);
    }

    // what if the value was actually NULL
    old_value = hashbucket_insert(bucket, hash, key, value);
    if (!old_value) hm->nitem++;
}

void *hashmap_remove(HashMap *hm, void *key) {
    HashBucket *bucket;
    hash_t hash;
    void *value;

    hash = hm->hash_func(key, hm->key_size);

    bucket = hashmap_get_bucket(hm, hash);
    if (!bucket) return NULL;

    value = hashbucket_remove(bucket, hash);
    if (value) hm->nitem--;

    return value;
}

void *hashmap_get(HashMap *hm, void *key) {
    HashBucket *bucket;
    HashNode *node;
    hash_t hash;

    hash = hm->hash_func(key, hm->key_size);

    bucket = hashmap_get_bucket(hm, hash);
    if (!bucket) return NULL;

    node = hashbucket_find(bucket, hash);

    return node ? node->value : NULL;
}