#include "hashmap.h"

static uint64_t make_hash(void *key) {
    return (uint64_t)key;
}

static inline void hashmap_free_entry(HashEntry *entry) {
    free(entry);
}

static void hashmap_free_entries(HashEntry *head) {
    HashEntry *curr, *next;

    if (!head)
        return;

    curr = head;
    next = curr->next;

    while (next) {
        curr = next;
        next = next->next;
        hashmap_free_entry(curr);
    }

    hashmap_free_entry(head);
}

static inline HashBucket *hashmap_get_bucket(HashMap *hm, uint64_t hash) {
    size_t pos;

    if (!hm->nbucket)
        return NULL;

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

static HashBucket *hashmap_new_bucket(HashMap *hm) {
    HashBucket *bucket;

    if (hm->nbucket)
        hm->buckets = realloc(hm->buckets, (hm->nbucket + 1) * sizeof(HashBucket));
    else
        hm->buckets = malloc(sizeof(HashBucket));
    hm->nbucket++;

    bucket = &hm->buckets[hm->nbucket - 1];
    bucket->head = NULL;

    return bucket;
}

static void *hashmap_remove_entry(HashBucket *bucket, uint64_t hash)
{
    HashEntry *curr, *prev;
    void *value = NULL;

    if (!bucket)
        return;

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

static HashEntry *hashmap_find_entry(HashBucket *bucket, uint64_t hash)
{
    HashEntry *entry;

    if (!bucket)
        return NULL;

    entry = bucket->head;
    while (entry && hash != entry->node.hash)
        entry = entry->next;

    return entry;
}

void hashmap_new(HashMap *hm) {
    memset(hm, 0, sizeof(HashMap));
}

void hashmap_free(HashMap *hm) {
    while (hm->nbucket--)
        hashmap_free_entries(hm->buckets[hm->nbucket].head);

    if (hm->buckets)
        free(hm->buckets);

    memset(hm, 0, sizeof(HashMap));
}

void *hashmap_insert(HashMap *hm, void *key, void *value) {
    HashBucket *bucket;
    HashEntry *entry;
    uint64_t hash;
    void *old_value = NULL;

    hash = make_hash(key);
    bucket = hashmap_get_bucket(hm, hash);

    if (!bucket)
        bucket = hashmap_new_bucket(hm);

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
    uint64_t hash;
    void *value;

    hash = make_hash(key);
    bucket = hashmap_get_bucket(hm, hash);
    value = hashmap_remove_entry(bucket, hash);
    
    if (value)
        hm->nitem--;

    return value;
}

void *hashmap_get(HashMap *hm, void *key) {
    HashBucket *bucket;
    HashEntry *entry;
    uint64_t hash;

    hash = make_hash(key);
    bucket = hashmap_get_bucket(hm, hash);
    entry = hashmap_find_entry(bucket, hash);

    return entry ? entry->node.value : NULL;
}