#ifndef QUERIES_UTILS_H
#define QUERIES_UTILS_H

#include "defn.h"
#include "index.h"
#include "results.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------//
/*
 * This structure holds a set of queries
 */
typedef struct QuerySet QuerySet;

/*
 * This structure holds a queries bitmap
 */
typedef struct QueriesBitMap QueriesBitMap;

//----------------------------------------------------------------------------//

// create all queries sets
void CreateQueriesSets(Queries *queries);

// destroy all queries sets
void DestroyQueriesSets(Queries *queries);

//----------------------------------------------------------------------------//

// create a particular query set
void CreateQuerySet(QuerySet *qtable);

// add a query into the given query set
void AddQuery(QuerySet *qtable, Query *query);

// destroy a particular query set
void DestroyQuerySet(QuerySet *qtable);

//----------------------------------------------------------------------------//

// given an query set deduplicate all queries - create entries based on
// the queries words - put them all into the entries entrylist
void DeduplicateQueries(QuerySet *qset, EntryList *entries);

//----------------------------------------------------------------------------//

//------------------- BIT MAP OPERATIONS -------------------------------------//
//----------------------------------------------------------------------------//

// create a bit map given the query set that holds all the queries
void CreateQueriesBitMap(QueriesBitMap *queries_bitmap, QuerySet general);

// given the bitmap fill it depending on entries payloads and dynamicaly
// fill the results of the query ids that match the current document
int UpdateBitMapAndFindResults(QueriesBitMap *queries_bitmap,
                               Results_struct *results, EntryList entries,
                               int distance);

// print the bitmap
void PrintQueriesBitMap(QueriesBitMap queries_bitmap, int total_queries);

// destroy the bitmap
void DestroyQueriesBitMap(QueriesBitMap *queries_bitmap, int total_queries);

// search the bitmap and return results - this function uses all above
unsigned int SearchQueriesBitMap(Results_struct *results,
                                 EntryList *hamming_results,
                                 EntryList *edit_results,
                                 EntryList exact_results, QuerySet general);

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//

#ifdef __cplusplus
}
#endif

#endif // QUERIES_UTILS_H