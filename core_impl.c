#include "core.h"
#include "defn_impl.h"
#include "index_impl.h"
#include "indexes.h"
#include "queries_utils_impl.h"
#include "results_impl.h"
#include "threadpool_impl.h"
#include "vp_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Indexes indexes;
static Queries queries;
static ResultsTable restable;

static int index_flag = 1;
static int gdoc_id_r = 1;

int globalfs = 0;
Thread_Pool *threadpool;
pthread_mutex_t results_mutex = PTHREAD_MUTEX_INITIALIZER;

enum ErrorCode InitializeIndex() {
    InitializeIndexes(&indexes);
    CreateQueriesSets(&queries);
    CreateResultsTable(&restable);
    threadpool = Initialize_ThPool(NUM_OF_THREADS);
    return EC_SUCCESS;
}

enum ErrorCode DestroyIndex() {
    DestroyIndexes(&indexes);
    DestroyQueriesSets(&queries);
    DestroyResultsTable(&restable);
    ThPool_Destroy(threadpool);
    return EC_SUCCESS;
}

enum ErrorCode StartQuery(QueryID query_id, const char *query_str,
                          enum MatchType match_type, unsigned int match_dist) {
    Query *query;
    query = malloc(sizeof(Query));

    if (query == NULL) {
        printf("Error: malloc() at 'StartQuery'\n");
        fflush(stdout);
    }

    CreateQuery(query_id, query_str, match_type, match_dist, query);

    if (match_type == MT_EXACT_MATCH)
        AddQuery(&queries.exact, query); // exact query set

    if (match_type == MT_HAMMING_DIST)
        AddQuery(&queries.hamming, query); // hamming query set

    if (match_type == MT_EDIT_DIST)
        AddQuery(&queries.edit, query); // edit query set

    AddQuery(&queries.general, query); // general query set - holds all above

    return EC_SUCCESS;
}

void ThreadMatchDocument(match_data *args) {
    DocID doc_id = args->doc_id;
    char *doc_str = args->doc_str;
    int c_index = args->c_index;

    Document doc;
    // create document and dynamicaly deduplicate it
    CreateDocument(doc_id, doc_str, &doc);

    EntryList hamming_results[4]; // hamming entrylists with dist 0,1,2,3
    EntryList edit_results[4];    // edit entrylists with dist 0,1,2,3
    EntryList exact_results;      // exact entrylist with dist 0

    // search all indexes and fill the results entrylists
    SearchIndexes(hamming_results, edit_results, &exact_results, doc, indexes,
                  c_index);

    Results_struct results_s;
    results_s.id_results = malloc(queries.general.size * sizeof(QueryID));

    if (results_s.id_results == NULL) {
        printf("Error: malloc() at 'ThreadMatchDocument' [id_results\n");
        fflush(stdout);
    }

    results_s.cur = 0;
    unsigned int id_results_size;

    // create bitmap and parse through it - update it
    // id_results will be updated to the query ids that match the doc
    id_results_size =
        SearchQueriesBitMap(&results_s, hamming_results, edit_results,
                            exact_results, queries.general);

    if (id_results_size)
        radixsort((int *)results_s.id_results, id_results_size); // sort results
    else
        free(results_s.id_results);

    DestroyDocument(&doc);
    free(args->doc_str);
    free(args);

    pthread_mutex_lock(&results_mutex);
    AddResult(&restable, results_s.id_results, id_results_size, doc_id - 1);
    pthread_mutex_unlock(&results_mutex);
}

enum ErrorCode MatchDocument(DocID doc_id, const char *doc_str) {
    if (index_flag) { // build all indexes
        BuildIndexes(&indexes, &queries);
        index_flag = 0;
    }

    globalfs = 0;
    match_data *args = malloc(sizeof(match_data));
    args->doc_str = malloc(strlen(doc_str) + 1);
    args->doc_id = doc_id;
    args->c_index = -1;

    if (args == NULL) {
        printf("Error: malloc() at 'MatchDocument [args]'\n");
        fflush(stdout);
    }

    if (args->doc_str == NULL) {
        printf("Error: malloc() at 'MatchDocument [args -> doc_str]'\n");
        fflush(stdout);
    }

    strcpy((char *)(args->doc_str), doc_str);
    // add job to thread pool jobqueue
    ThPool_AddJob(threadpool, (void *)ThreadMatchDocument, (void *)args);
    return EC_SUCCESS;
}

enum ErrorCode GetNextAvailRes(DocID *p_doc_id, unsigned int *p_num_res,
                               QueryID **p_query_ids) {
    if (!globalfs) {             // if this is the first GetNextAvailRes
        ThPool_Wait(threadpool); // wait all match docs to terminate
        globalfs = 1;
    }

    GetResults(&restable, p_query_ids, p_num_res, gdoc_id_r - 1);
    *p_doc_id = gdoc_id_r; // point to the current document id
    gdoc_id_r++;           // get ready for the next doc id
    return EC_SUCCESS;
}
