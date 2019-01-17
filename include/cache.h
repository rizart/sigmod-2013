#ifndef CACHE_H
#define CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "defn.h"
#include "index.h"

// this is the queue size of the cache
#define CACHE_SIZE 100000

// this structure holds a cache entry
typedef struct Cache_Entry Cache_Entry;

// this structure holds the cache queue
typedef struct Cache_Queue Cache_Queue;

// this structure holds the cache
typedef struct Cache Cache;

// initialize cache given the cache size
void Initialize_Cache(Cache *cache, int queue_size);

// destroy cache
void Destroy_Cache(Cache *cache);

// search cache to find the given word
// return results if found in edit_results entrylist
// mark the bucket of tha hashtable that we may need later
int Search_Cache(Cache *cache, Word word, EntryList *edit_results,
                 unsigned int *lru_bucket);

// update cache with the given word and the given entrylist
// lru bucket makes update faster indicating the place where the update
// will take place
// if found_in_cache is true there will be no update at all
void Update_Cache(Cache *cache, Word word, EntryList *edit_results,
                  unsigned int lru_bucket, int found_in_cache);

#ifdef __cplusplus
}
#endif

#endif /* CACHE_H */