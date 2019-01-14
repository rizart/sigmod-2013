#include "cache_impl.h"
#include "defn_impl.h"
#include "fnv.h"
#include "index_impl.h"
#include <stdio.h>
#include <stdlib.h>

//------------------------ CACHE QUEUE OPERATIONS ----------------------------//

void Initialize_CacheQueue(Cache *cache) {
    cache->queue->head = NULL;        // theres not head
    cache->queue->tail = NULL;        // nor tail
    cache->queue->num_of_entries = 0; // nore entries
}

void LRU_CacheQueue(Cache *cache, int lru_bucket, Word word) {
    Cache_Entry *tail;
    tail = cache->queue->tail;

    // if cache entry is already where it must go we are good
    if (!Equal(&tail->word, &word) || cache->queue->num_of_entries == 1)
        return;

    Cache_Entry *cur;

    // we will search the entry via the hashtable - it's faster
    cur = cache->HashTable[lru_bucket];

    while (cur != NULL) { // find the cache entry

        if (!Equal(&cur->word, &word)) { // we found the entry

            cur->prev->next = cur->next;

            if (cur->next != NULL) // if this not the head of the queue
                cur->next->prev = cur->prev;
            else
                cache->queue->head = cur->prev;

            tail->prev = cur;
            cur->next = tail;
            cur->prev = NULL;
            cache->queue->tail = cur;

            return;
        }

        cur = cur->hash_next;
    }
}

void Destroy_CacheQueue_Entry(Cache_Entry *centry) {
    int i;

    for (i = 0; i != 4; i++)
        DestroyEntryList(&centry->edit_results[i]);

    DestroyWord(&centry->word);
    free(centry);
}

void Destroy_CacheQueue(Cache *cache) {
    Cache_Entry *cur_entry;
    cur_entry = cache->queue->head;

    while (cache->queue->num_of_entries) {
        cache->queue->head = cur_entry->prev;

        Destroy_CacheQueue_Entry(cur_entry);

        cur_entry = cache->queue->head;
        cache->queue->num_of_entries--;
    }

    cache->queue->head = NULL;
    cache->queue->tail = NULL;
}

void AddEntry_CacheQueue(Cache *cache, Cache_Entry *centry) {
    centry->next = NULL;
    centry->prev = NULL;

    Cache_Entry *cur_last_entry;
    cur_last_entry = cache->queue->tail;

    int num_of_entries = cache->queue->num_of_entries;

    if (!num_of_entries) { // cache queue is empty
        cache->queue->head = centry;
        cache->queue->tail = centry;
    } else { // cache queue is not empty
        cur_last_entry->prev = centry;
        centry->next = cur_last_entry;
        cache->queue->tail = centry;
    }

    (cache->queue->num_of_entries)++; // increase num of entries
}

//---------------------- CACHE HASHTABLE OPERATIONS --------------------------//

void AddEntry_CacheHashTable(Cache *cache, unsigned int bucket,
                             Cache_Entry *centry) {
    if (cache->HashTable[bucket] == NULL) { // first entry
        cache->HashTable[bucket] = centry;
        cache->HashTable[bucket]->hash_next = NULL;
        cache->HashTable[bucket]->hash_prev = NULL;
    } else {
        Cache_Entry *cur_entry;
        cur_entry = cache->HashTable[bucket];

        while (cur_entry->hash_next != NULL)
            cur_entry = cur_entry->hash_next;

        cur_entry->hash_next = centry;
        centry->hash_prev = cur_entry;
        centry->hash_next = NULL;
    }
}

//-------------------------- CACHE OPERATIONS --------------------------------//

void Initialize_Cache(Cache *cache, int queue_size) {
    int hash_size = FindNextPrime(queue_size);
    int i;

    // initialize hashtable
    cache->HashTable = malloc(hash_size * sizeof(Cache_Entry *));
    for (i = 0; i != hash_size; i++)
        cache->HashTable[i] = NULL;
    cache->hash_size = hash_size;

    if (cache->HashTable == NULL) {
        printf("Error: malloc() at 'Initialize_Cache' [HashTable]\n");
        return;
    }

    // initialize cache queue
    cache->queue = malloc(sizeof(Cache_Queue));
    cache->queue_size = queue_size;

    if (cache->queue == NULL) {
        printf("Error: malloc() at 'Initialize_Cache' [queue]\n");
        return;
    }

    Initialize_CacheQueue(cache);
    cache->searches = 0;
    cache->hits = 0;
}

void RemoveFirstEntry(Cache *cache) {
    Cache_Entry *cur_first_entry;
    cur_first_entry = cache->queue->head;

    unsigned int hashprime = cache->hash_size;
    unsigned int bucket;
    bucket = FNV32(cur_first_entry->word.word_str, hashprime);

    // entry which is going to be removed is at the first pointer of hashtable
    if (!Equal(&cur_first_entry->word, &cache->HashTable[bucket]->word))
        cache->HashTable[bucket] = cache->HashTable[bucket]->hash_next;

    int num_of_entries = cache->queue->num_of_entries;

    if (num_of_entries == 1) { // cache queue has only one entry
        cache->queue->head = NULL;
        cache->queue->tail = NULL;
    } else { // cache queue has more than one entries
        cur_first_entry->prev->next = NULL;

        // hash table mods
        if (cur_first_entry->hash_prev !=
            NULL) // not the first entry pointed by hasht
            cur_first_entry->hash_prev->hash_next = cur_first_entry->hash_next;
        if (cur_first_entry->hash_next !=
            NULL) // not the last entry pointed by hasht
            cur_first_entry->hash_next->hash_prev = cur_first_entry->hash_prev;

        cache->queue->head = cur_first_entry->prev;
    }

    (cache->queue->num_of_entries)--; // decrease num of entries
    Destroy_CacheQueue_Entry(cur_first_entry);
}

void Update_Cache(Cache *cache, Word word, EntryList *edit_results,
                  unsigned int lru_bucket, int found_in_cache) {
    if (found_in_cache) { // LRU must take place

        LRU_CacheQueue(cache, lru_bucket, word);

    } else { // we need to put the entrylists in a new cache entry

        // no need for update - there's nothing to put in the cache
        if (!edit_results[0].size && !edit_results[1].size &&
            !edit_results[2].size && !edit_results[3].size)
            return;

        Cache_Entry *centry;
        int i;

        centry = malloc(sizeof(Cache_Entry));
        if (centry == NULL) {
            printf("Error: malloc() at 'Update_Cache' [centry]\n");
            return;
        }

        unsigned int hashprime = cache->hash_size;
        unsigned int bucket;
        bucket = FNV32(word.word_str, hashprime);

        for (i = 0; i != 4; i++)
            CopyEntryList(&centry->edit_results[i], &edit_results[i], 1);

        CreateWord(word.word_str, &centry->word);

        // we need to remove first entry in queue and put the new one at the end
        if (cache->queue->num_of_entries == cache->queue_size)
            RemoveFirstEntry(cache);

        AddEntry_CacheHashTable(cache, bucket, centry);
        AddEntry_CacheQueue(cache, centry);
    }
}

int Search_Cache(Cache *cache, Word word, EntryList *edit_results,
                 unsigned int *lru_bucket) {
    unsigned int hashprime = cache->hash_size;
    unsigned int bucket;
    int i;

    bucket = FNV32(word.word_str, hashprime);
    *lru_bucket = bucket;

    Cache_Entry *cur;

    cur = cache->HashTable[bucket];

    if (cur == NULL)
        return 0;
    else
        cache->searches++;

    while (cur != NULL) { // find the cache entry

        if (!Equal(&cur->word, &word)) { // entry found
            for (i = 0; i != 4; i++)
                CopyEntryList(&edit_results[i], &cur->edit_results[i], 0);
            cache->hits++;
            return 1;
        }

        cur = cur->hash_next;
    }

    return 0;
}

void Destroy_Cache(Cache *cache) {
    // printf("Cache hit ratio:%.2f%%\n",
    //        ((float)(cache->hits) / (float)(cache->searches)) * 100);
    free(cache->HashTable);
    Destroy_CacheQueue(cache);
    free(cache->queue);
}
