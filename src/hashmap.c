#include "hashmap.h"

//////////////////////////////////////////////////////////////////////////////////////
// other functions
//////////////////////////////////////////////////////////////////////////////////////

// Perl's hash function
static hash_t hash_func_generic(const void *key, size_t size) {
   register const uint8_t *data;
   register size_t         i;
   register hash_t         hash;

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
// HashNode
//////////////////////////////////////////////////////////////////////////////////////

inline static void hashnode_get(HashMap *map, HashNode *node, void *value) {
   memcpy(value, node->value, map->value_size);
}

//////////////////////////////////////////////////////////////////////////////////////
// HashBucket
//////////////////////////////////////////////////////////////////////////////////////

static void hashbucket_free(HashBucket *bucket) {
   while (bucket->len--)
      free(bucket->ptr[bucket->len].value);
   free(bucket->ptr);
   bucket->cap = bucket->len = 0;
}

inline static void hashbucket_grow(HashBucket *bucket) {
   if (bucket->len == bucket->cap) {
      if (bucket->cap) {
         bucket->cap = bucket->cap * 2;
         bucket->ptr = realloc(bucket->ptr, bucket->cap * sizeof(HashNode));
      }
      else {
         bucket->cap = 2;
         bucket->ptr = malloc(bucket->cap * sizeof(HashNode));
      }
   }
}

static HashNode *hashbucket_find(HashBucket *bucket, hash_t hash) {
   HashNode *node;

   for (size_t i = 0; i < bucket->len; i++) {
      node = &bucket->ptr[i];
      if (node->hash == hash && node->key != NULL) {
         return node;
      }
   }

   return NULL;
}

static void hashbucket_push(HashMap *map, HashBucket *bucket, hash_t hash, void *key, void *value) {
   HashNode *node;

   hashbucket_grow(bucket);
   node = &bucket->ptr[bucket->len];
   bucket->len++;

   node->hash = hash;
   node->key = key;
   node->value = malloc(map->value_size);
   memcpy(node->value, value);
}

static bool hashbucket_insert(
   HashMap    *map,
   HashBucket *bucket,
   hash_t      hash,
   void       *key,
   void       *value,
   void       *prev
) {
   HashNode *found;

   found = hashbucket_find(bucket, hash);
   if (!found)
      hashbucket_push(map, bucket, hash, key, value);
   else {
      if (prev)
         memcpy(prev, found->value, map->value_size);
      memcpy(found->value, value, map->value_size);
   }

   return found != NULL;
}

static bool hashbucket_remove(HashMap *map, HashBucket *bucket, hash_t hash, void *prev) {
   HashNode *node;

   node = hashbucket_find(bucket, hash);
   if (!node)
      return false;

   if (prev)
      memcpy(prev, node->value, map->value_size);
   node->key = NULL;

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
   }
   else {
      map->nbucket = 8;
      map->buckets = calloc(map->nbucket, sizeof(HashBucket));
   }
}

inline static HashBucket *hashmap_get_bucket(HashMap *map, hash_t hash) {
   size_t pos;

   if (!map->nbucket)
      return NULL;
   // since nbucket is always a power of 2, i don't need to use modulo
   pos = hash & (map->nbucket - 1);

   return &map->buckets[pos];
}

void hashmap_new(HashMap *map, size_t key_size, size_t value_size) {
   map->buckets = NULL;
   map->nbucket = map->nitem = 0;

   map->key_size = key_size;
   map->value_size = value_size;

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
   HashNode   *node;
   hash_t      hash;

   hash = map->hash_func(key, map->key_size);

   bucket = hashmap_get_bucket(map, hash);
   if (!bucket) {
      hashmap_grow(map);
      bucket = hashmap_get_bucket(map, hash);
   }

   bool found = hashbucket_insert(map, bucket, hash, key, value, prev);
   if (!found)
      map->nitem++;

   return found;
}

bool hashmap_remove(HashMap *map, void *key, void *value) {
   HashBucket *bucket;
   hash_t      hash;

   hash = map->hash_func(key, map->key_size);

   bucket = hashmap_get_bucket(map, hash);
   if (!bucket)
      return false;

   bool found = hashbucket_remove(map, bucket, hash, value);
   if (found)
      map->nitem--;

   return found;
}

bool hashmap_get(HashMap *map, void *key, void *value) {
   HashBucket *bucket;
   HashNode   *node;
   hash_t      hash;

   hash = map->hash_func(key, map->key_size);

   bucket = hashmap_get_bucket(map, hash);
   if (!bucket)
      return false;

   node = hashbucket_find(bucket, hash);
   if (!node)
      return false;

   hashnode_get(map, node, value);

   return true;
}