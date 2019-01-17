#include "cache_impl.h"
#include "core.h"
#include "defn_impl.h"
#include "fnv.h"
#include "index_impl.h"
#include "queries_utils_impl.h"
#include "threadpool.h"
#include "vp_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Cache cache[NUM_OF_THREADS];

// INDEXES

void InitializeIndexes(Indexes *indexes) {
    InitializeEditIndex(&indexes->edit);
    InitializeHammingIndex(&indexes->hamming);
    InitializeExactIndex(&indexes->exact);

    int i;
    for (i = 0; i != NUM_OF_THREADS; i++)
        Initialize_Cache(&cache[i], CACHE_SIZE);
}

void DestroyIndexes(Indexes *indexes) {
    DestroyEditIndex(&indexes->edit);
    DestroyHammingIndex(&indexes->hamming);
    DestroyExactIndex(&indexes->exact);

    int i;
    for (i = 0; i != NUM_OF_THREADS; i++)
        Destroy_Cache(&cache[i]);
}

void BuildIndexes(Indexes *indexes, Queries *queries) {
    EntryList edit_entries;
    EntryList hamming_entries;
    EntryList exact_entries;

    CreateEntryList(&edit_entries);
    DeduplicateQueries(&queries->edit, &edit_entries);
    if (edit_entries.size)
        BuildEditIndex(&indexes->edit, &edit_entries);
    DestroyEntryList(&edit_entries);

    CreateEntryList(&exact_entries);
    DeduplicateQueries(&queries->exact, &exact_entries);
    if (&exact_entries.size)
        ;
    BuildExactIndex(&indexes->exact, &exact_entries);
    DestroyEntryList(&exact_entries);

    CreateEntryList(&hamming_entries);
    DeduplicateQueries(&queries->hamming, &hamming_entries);
    if (hamming_entries.size)
        BuildHammingIndex(&indexes->hamming, &hamming_entries);
    DestroyEntryList(&hamming_entries);
}

void SearchIndexes(EntryList *hamming_results, EntryList *edit_results,
                   EntryList *exact_results, Document doc, Indexes indexes,
                   int c) {
    int i, j;

    for (i = 0; i != 4; i++) {
        CreateEntryList(&hamming_results[i]);
        CreateEntryList(&edit_results[i]);
    }
    CreateEntryList(exact_results);

    int docsize = doc.num_of_words;

    int found_in_cache = 0;
    unsigned int bucket;

    for (i = 0; i != docsize; i++) { // for every word search all indexes

        found_in_cache =
            Search_Cache(&cache[c], *doc.words[i], edit_results, &bucket);

        EntryList edit_results_cache[4]; // edit entrylists with dist 0,1,2,3
        for (j = 0; j != 4; j++)
            CreateEntryList(&edit_results_cache[j]);

        if (!found_in_cache) // word was not found in cache
            SearchEditIndex(indexes.edit, *doc.words[i], edit_results,
                            edit_results_cache);

        SearchExactIndex(indexes.exact, *doc.words[i], exact_results);

        SearchHammingIndex(indexes.hamming, *doc.words[i], hamming_results);

        Update_Cache(&cache[c], *doc.words[i], edit_results_cache, bucket,
                     found_in_cache);

        for (j = 0; j != 4; j++)
            DestroyEntryList(&edit_results_cache[j]);
    }
}

// EDIT DISTANCE INDEX

void InitializeEditIndex(Edit_Index *index) {
    InitializeVpTree(&index->vp_tree);
}

void DestroyEditIndex(Edit_Index *index) {
    DestroyVpTree(index->vp_tree.root);
}

void BuildEditIndex(Edit_Index *index, EntryList *entries) {
    BuildVpTree(index->vp_tree.root, entries, MT_EDIT_DIST);
}

void SearchEditIndex(Edit_Index index, Word word, EntryList *result,
                     EntryList *result_cache) {
    int i;

    for (i = 0; i != 4; i++)
        SearchVpTree(&word, i, index.vp_tree.root, &result[i], &result_cache[i],
                     MT_EDIT_DIST);
}

// HAMMING DISTANCE INDEX

void InitializeHammingIndex(Hamming_Index *index) {
    int i = 0;
    int vp_table_size = MAX_WORD_LENGTH - MIN_WORD_LENGTH + 1;

    for (i = 0; i != vp_table_size; i++)
        InitializeVpTree(&index->vp_table[i]);
}

void DestroyHammingIndex(Hamming_Index *index) {
    int i = 0;
    int vp_table_size = MAX_WORD_LENGTH - MIN_WORD_LENGTH + 1;

    for (i = 0; i != vp_table_size; i++)
        DestroyVpTree(index->vp_table[i].root);
}

void BuildHammingIndex(Hamming_Index *index, EntryList *entries) {
    int i;
    int entries_size = entries->size;
    int table_size = MAX_WORD_LENGTH - MIN_WORD_LENGTH + 1;

    EntryList ElistTable[table_size];
    for (i = 0; i != table_size; i++)
        CreateEntryList(&ElistTable[i]);

    for (i = 0; i != entries_size; i++) {
        // according to length create entrylists
        int length = strlen(entries->elist[i].word->word_str);
        AddEntry(&ElistTable[length - MIN_WORD_LENGTH], &entries->elist[i]);
    }

    for (i = 0; i != table_size; i++) {
        if (ElistTable[i].size) // build vp trees
            BuildVpTree(index->vp_table[i].root, &ElistTable[i],
                        MT_HAMMING_DIST);
        DestroyEntryList(&ElistTable[i]);
    }
}

void SearchHammingIndex(Hamming_Index index, Word word, EntryList *result) {
    int length = strlen(word.word_str);
    int i;

    for (i = 0; i != 4; i++)
        SearchVpTree(&word, i, index.vp_table[length - MIN_WORD_LENGTH].root,
                     &result[i], NULL, MT_HAMMING_DIST);
}

// EXACT MATCH INDEX

void InitializeExactIndex(Exact_Index *index) {
    index->HashTable = NULL;
    index->hashtable_size = 0;
}

void BuildExactIndex(Exact_Index *index, EntryList *entries) {
    int i, bucket;
    int hash_prime = FindNextPrime(entries->size);
    int size = hash_prime;

    index->hashtable_size = size;
    index->HashTable = malloc(size * sizeof(BTree_Node *));

    if (index->HashTable == NULL) {
        printf("Error: malloc() at 'BuildExactIndex'\n");
        fflush(stdout);
    }

    for (i = 0; i != size; i++)
        index->HashTable[i] = NULL;

    int esize = entries->size;

    for (i = 0; i != esize; i++) {
        bucket = FNV32(entries->elist[i].word->word_str, hash_prime);

        index->HashTable[bucket] =
            InsertEntryInBTree(index->HashTable[bucket], entries->elist[i]);
    }
}

void SearchExactIndex(Exact_Index index, Word word, EntryList *results) {
    int size = index.hashtable_size;

    int bucket = FNV32(word.word_str, size);

    SearchBTree(index.HashTable[bucket], word, results);
}

void DestroyExactIndex(Exact_Index *index) {
    int i;
    int size = index->hashtable_size;

    for (i = 0; i != size; i++) {
        if (index->HashTable[i] != NULL)
            DestroyBTree(index->HashTable[i]);

        index->HashTable[i] = NULL;
    }

    if (index->HashTable != NULL)
        free(index->HashTable);
}
