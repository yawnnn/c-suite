#include "hashmap.h"

//////////////////////////////////////////////////////////////////////////////////////
// useful functions
//////////////////////////////////////////////////////////////////////////////////////

static inline char *offset(void *ptr, size_t size, size_t count) {
    return (char *)ptr + (size * count);
}

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

//////////////////////////////////////////////////////////////////////////////////////
// HashData
//////////////////////////////////////////////////////////////////////////////////////

#define IDX_DATA_NULL ((size_t) - 1)

static void hashdata_free(HashData *data) {
    free(data->keys);
    free(data->values);
}

static inline void hashdata_grow(HashMap *map, HashData *data) {
    if (data->len == data->cap) {
        if (data->cap) {
            data->cap = data->cap * 2;
            data->keys = realloc(data->keys, data->cap * map->key_size);
            data->values = realloc(data->values, data->cap * map->value_size);
        } else {
            data->cap = 2;
            data->keys = malloc(data->cap * map->key_size);
            data->values = malloc(data->cap * map->value_size);
        }
    }
}

static size_t
hashdata_push(HashMap *map, HashData *data, void *key, void *value) {
    char *pkey, *pvalue;

    hashdata_grow(map, data);
    pkey = offset(data->keys, map->key_size, data->len);
    memcpy(pkey, key, map->key_size);
    pvalue = offset(data->values, map->value_size, data->len);
    memcpy(pvalue, value, map->value_size);
    data->len++;

    return data->len - 1;
}

static inline void
hashdata_get(HashMap *map, HashData *data, size_t pos, void *value) {
    char *pvalue;

    if (pos < data->len) {
        pvalue = offset(data->values, map->value_size, pos);
        memcpy(value, pvalue, map->value_size);
    }
}

static inline void
hashdata_set(HashMap *map, HashData *data, size_t pos, void *value) {
    char *pvalue;

    if (pos < data->len) {
        pvalue = offset(data->values, map->value_size, pos);
        memcpy(pvalue, value, map->value_size);
    }
}

static void
hashdata_remove(HashMap *map, HashData *data, size_t pos, void *value) {
    if (value) hashdata_get(map, data, pos, value);
    // TODO --- Uhm...
}

//////////////////////////////////////////////////////////////////////////////////////
// HashBucket
//////////////////////////////////////////////////////////////////////////////////////

static void hashbucket_free(HashBucket *bucket) {
    free(bucket->ptr);
    bucket->cap = bucket->len = 0;
}

static inline void hashbucket_grow(HashBucket *bucket) {
    if (bucket->len == bucket->cap) {
        if (bucket->cap) {
            bucket->cap = bucket->cap * 2;
            bucket->ptr = realloc(bucket->ptr, bucket->cap * sizeof(HashNode));
        } else {
            bucket->cap = 2;
            bucket->ptr = malloc(bucket->cap * sizeof(HashNode));
        }
    }
}

static void hashbucket_push(HashBucket *bucket, hash_t hash, size_t idx) {
    hashbucket_grow(bucket);
    bucket->ptr[bucket->len].hash = hash;
    bucket->ptr[bucket->len].idx = idx;
}

static HashNode *hashbucket_find(HashBucket *bucket, hash_t hash) {
    HashNode *node;

    for (size_t i = 0; i < bucket->len; i++) {
        node = &bucket->ptr[i];
        if (node->hash == hash && node->idx != IDX_DATA_NULL) {
            return node;
        }
    }

    return NULL;
}

static bool hashbucket_insert(
    HashMap *map,
    HashBucket *bucket,
    hash_t hash,
    void *key,
    void *value,
    void *prev
) {
    HashNode *node;
    size_t idx;
    bool found;

    node = hashbucket_find(bucket, hash);
    if (node) {
        if (prev) hashdata_get(map, &map->data, node->idx, prev);
        hashdata_set(map, &map->data, node->idx, value);
        found = true;
    } else {
        idx = hashdata_push(map, &map->data, key, value);
        hashbucket_push(bucket, hash, idx);
        found = false;
    }

    return found;
}

static bool
hashbucket_remove(HashMap *map, HashBucket *bucket, hash_t hash, void *prev) {
    HashNode *node;

    node = hashbucket_find(bucket, hash);
    if (!node) return false;

    hashdata_remove(map, &map->data, node->idx, prev);
    node->idx = IDX_DATA_NULL;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////
// HashMap
//////////////////////////////////////////////////////////////////////////////////////

static void hashmap_grow(HashMap *map) {
    // nbucket is always kept at a power of 2
    if (map->nbucket) {
        size_t old_nbucket = map->nbucket;

        map->nbucket = map->nbucket * 2;
        map->buckets = realloc(map->buckets, map->nbucket * sizeof(HashBucket));
        memset(&map->buckets[old_nbucket], 0, old_nbucket * sizeof(HashBucket));
    } else {
        map->nbucket = 8;
        map->buckets = calloc(map->nbucket, sizeof(HashBucket));
    }
}

static inline HashBucket *hashmap_get_bucket(HashMap *map, hash_t hash) {
    size_t pos;

    if (!map->nbucket) return NULL;

    // should be faster than modulo
    // possible because nbucket is always a power of 2
    pos = hash & (map->nbucket - 1);

    return &map->buckets[pos];
}

void hashmap_new(HashMap *map, size_t key_size, size_t value_size) {
    map->buckets = NULL;
    map->nbucket = map->nitem = 0;

    map->key_size = key_size;

    map->hash_func = hash_func_generic;
}

void hashmap_free(HashMap *map) {
    while (map->nbucket--)
        hashbucket_free(&map->buckets[map->nbucket]);
    free(map->buckets);
    memset(map, 0, sizeof(HashMap));
}

bool hashmap_insert(HashMap *map, void *key, void *value, void *prev) {
    HashBucket *bucket;
    HashNode *node;
    hash_t hash;

    hash = map->hash_func(key, map->key_size);

    bucket = hashmap_get_bucket(map, hash);
    if (!bucket) {
        hashmap_grow(map);
        bucket = hashmap_get_bucket(map, hash);
    }

    bool found = hashbucket_insert(map, bucket, hash, key, value, prev);
    if (!found) map->nitem++;

    return found;
}

bool hashmap_remove(HashMap *map, void *key, void *value) {
    HashBucket *bucket;
    hash_t hash;

    hash = map->hash_func(key, map->key_size);

    bucket = hashmap_get_bucket(map, hash);
    if (!bucket) return false;

    bool found = hashbucket_remove(map, bucket, hash, value);
    if (found) map->nitem--;

    return found;
}

bool hashmap_get(HashMap *map, void *key, void *value) {
    HashBucket *bucket;
    HashNode *node;
    hash_t hash;

    hash = map->hash_func(key, map->key_size);

    bucket = hashmap_get_bucket(map, hash);
    if (!bucket) return false;

    node = hashbucket_find(bucket, hash);
    if (!node) return false;

    hashdata_get(map, &map->data, node->idx, value);

    return true;
}