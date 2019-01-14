#include "binary_tree_impl.h"
#include "defn_impl.h"
#include "fnv.h"
#include "index_impl.h"
#include "queries_utils_impl.h"
#include "results_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void CreateQueriesSets(Queries *queries) {
    // create all query sets
    CreateQuerySet(&queries->edit);
    CreateQuerySet(&queries->hamming);
    CreateQuerySet(&queries->exact);
    CreateQuerySet(&queries->general);
}

void DestroyQueriesSets(Queries *queries) {
    // destroy all query sets
    DestroyQuerySet(&queries->edit);
    DestroyQuerySet(&queries->hamming);
    DestroyQuerySet(&queries->exact);
    free(queries->general.table);
}

void CreateQuerySet(QuerySet *qtable) {
    qtable->max_size = 512;
    qtable->size = 0;
    qtable->total_qset_words = 0;
    qtable->table = malloc(qtable->max_size * sizeof(Query *));

    if (qtable->table == NULL) {
        printf("Error: malloc() at 'CreateQuerySet'\n");
        fflush(stdout);
    }
}

void AddQuery(QuerySet *qtable, Query *query) {
    int size = qtable->size;
    int max_size = qtable->max_size;

    if (size == max_size) { // we need reallocation
        qtable->max_size *= 2;
        qtable->table =
            realloc(qtable->table, qtable->max_size * sizeof(Query *));
        if (qtable->table == NULL) {
            printf("Error: realloc() at 'AddQuery'\n");
            fflush(stdout);
        }
    }

    qtable->table[size] = query;
    qtable->total_qset_words += query->num_of_words;
    qtable->size++;
}

void DestroyQuerySet(QuerySet *qtable) {
    int size = qtable->size;
    int i = 0;

    for (i = 0; i != size; i++) {
        DestroyQuery(qtable->table[i]);
        free(qtable->table[i]);
    }
    free(qtable->table);
    qtable->table = NULL;
}

void DeduplicateQueries(QuerySet *qset, EntryList *entries) {
    int i, j;
    int size = qset->size;

    int hashprime = FindNextPrime(qset->total_qset_words);
    unsigned int bucket;

    BTree_Node *HashTable[hashprime];

    for (i = 0; i != hashprime; i++)
        HashTable[i] = NULL;

    for (i = 0; i != size; i++) { // for every query inside the query set

        int query_size = qset->table[i]->num_of_words;
        int query_id = qset->table[i]->id;

        for (j = 0; j != query_size; j++) { // for every word of this query

            // hash the query word
            bucket = FNV32(qset->table[i]->word[j]->word_str, hashprime);

            Entry entry;
            CreateEntry(qset->table[i]->word[j], &entry);
            UpdatePayload(&entry, query_id, j);

            HashTable[bucket] = InsertEntryInBTree(HashTable[bucket], entry);

            DestroyEntry(&entry);
        }
    }

    for (i = 0; i != hashprime; i++)
        if (HashTable[i] != NULL) {
            ExtractEntriesFromBTree(HashTable[i], entries);
            DestroyBTree(HashTable[i]);
        }
}

//------------------- BIT MAP OPERATIONS -------------------------------------//

void CreateQueriesBitMap(QueriesBitMap *queries_bitmap, QuerySet general) {
    int total_queries = general.size;
    int i, j;

    queries_bitmap->bitmap = malloc(total_queries * sizeof(char *));
    queries_bitmap->distance = malloc(total_queries * sizeof(int));
    queries_bitmap->counter = malloc(total_queries * sizeof(int));
    queries_bitmap->total_qwords = malloc(total_queries * sizeof(int));

    if (queries_bitmap->bitmap == NULL) {
        printf("Error: malloc() at 'CreateQueriesBitMap' [bitmap]\n");
        fflush(stdout);
    }
    if (queries_bitmap->distance == NULL) {
        printf("Error: malloc() at 'CreateQueriesBitMap' [distance]\n");
        fflush(stdout);
    }
    if (queries_bitmap->counter == NULL) {
        printf("Error: malloc() at 'CreateQueriesBitMap' [counter]\n");
        fflush(stdout);
    }
    if (queries_bitmap->total_qwords == NULL) {
        printf("Error: malloc() at 'CreateQueriesBitMap' [total_qwords]\n");
        fflush(stdout);
    }

    for (i = 0; i != total_queries; i++) { // for every query

        queries_bitmap->bitmap[i] = malloc(MAX_QUERY_WORDS * sizeof(char));

        if (queries_bitmap->bitmap[i] == NULL) {
            printf("Error: malloc() at 'CreateQueriesBitMap' [bitmap[i]]\n");
            fflush(stdout);
        }

        // set distance , num of words and a counter initialized to zero
        queries_bitmap->distance[i] = general.table[i]->match_dist;
        queries_bitmap->total_qwords[i] = general.table[i]->num_of_words;
        queries_bitmap->counter[i] = 0;

        // initialize bitmap
        for (j = 0; j != MAX_QUERY_WORDS; j++) {
            if (j < general.table[i]->num_of_words)
                queries_bitmap->bitmap[i][j] = 0;
            else
                queries_bitmap->bitmap[i][j] = 9;
        }
    }
}

int UpdateBitMapAndFindResults(QueriesBitMap *queries_bitmap,
                               Results_struct *results, EntryList entries,
                               int distance) {
    int esize = entries.size;
    int i, j;
    int counter = 0;

    for (i = 0; i != esize; i++) { // for every entry inside the entrylist

        int payload_size = entries.elist[i].payload.size;

        for (j = 0; j != payload_size; j++) { // take the payload

            int qid = entries.elist[i].payload.table[j].qid;
            int pos = entries.elist[i].payload.table[j].position;

            // we have a match
            if (!queries_bitmap->bitmap[qid - 1][pos] &&
                queries_bitmap->distance[qid - 1] == distance) {
                queries_bitmap->bitmap[qid - 1][pos] = 1;
                queries_bitmap->counter[qid - 1]++;

                // if counter reached query words then this is a result
                if (queries_bitmap->counter[qid - 1] ==
                    queries_bitmap->total_qwords[qid - 1]) {
                    // update results
                    int cur = results->cur;
                    results->id_results[cur] = qid;
                    results->cur++;
                    counter++;
                }
            }
        }
    }

    return counter;
}

unsigned int SearchQueriesBitMap(Results_struct *results,
                                 EntryList *hamming_results,
                                 EntryList *edit_results,
                                 EntryList exact_results, QuerySet general) {
    unsigned int id_results_size = 0;
    int i;

    QueriesBitMap queries_bitmap;
    CreateQueriesBitMap(&queries_bitmap, general);

    for (i = 0; i != 4; i++) { // for every type of match dist entrylist

        id_results_size += UpdateBitMapAndFindResults(&queries_bitmap, results,
                                                      hamming_results[i], i);
        DestroyEntryList(&hamming_results[i]);

        id_results_size += UpdateBitMapAndFindResults(&queries_bitmap, results,
                                                      edit_results[i], i);
        DestroyEntryList(&edit_results[i]);
    }

    id_results_size +=
        UpdateBitMapAndFindResults(&queries_bitmap, results, exact_results, 0);
    DestroyEntryList(&exact_results);

    DestroyQueriesBitMap(&queries_bitmap, general.size);

    return id_results_size;
}

void DestroyQueriesBitMap(QueriesBitMap *queries_bitmap, int total_queries) {
    int i;
    for (i = 0; i != total_queries; i++)
        free(queries_bitmap->bitmap[i]);
    // free all memory
    free(queries_bitmap->distance);
    free(queries_bitmap->counter);
    free(queries_bitmap->bitmap);
    free(queries_bitmap->total_qwords);
}

void PrintQueriesBitMap(QueriesBitMap queries_bitmap, int total_queries) {
    int i, j;

    for (i = 0; i != total_queries; i++) {
        for (j = 0; j != MAX_QUERY_WORDS; j++)
            printf("%d ", (int)queries_bitmap.bitmap[i][j]);

        printf("|  %d  |", queries_bitmap.distance[i]);
        printf(" %d | ", queries_bitmap.counter[i]);
        printf(" %d |\n", queries_bitmap.total_qwords[i]);
    }
}
