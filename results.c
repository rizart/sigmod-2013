#include "results_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void CreateResultsTable(ResultsTable *restable) {
    restable->max_size = 1024;
    restable->size = 0;
    restable->id_results = malloc(restable->max_size * sizeof(QueryID *));
    restable->sizes = malloc(restable->max_size * sizeof(int));

    if (restable->id_results == NULL) {
        printf("Error: malloc() at 'CreateResultsTable' [id_results]\n");
        fflush(stdout);
    }
    if (restable->sizes == NULL) {
        printf("Error: malloc() at 'CreateResultsTable' [sizes]\n");
        fflush(stdout);
    }
}

void AddResult(ResultsTable *restable, QueryID *results, int sizel, int pos) {
    int size = restable->size;
    int max_size = restable->max_size;

    if (size == max_size || pos + 1 > max_size) { // we need reallocation

        restable->max_size *= 2;
        restable->id_results = realloc(restable->id_results,
                                       restable->max_size * sizeof(QueryID *));
        restable->sizes =
            realloc(restable->sizes, restable->max_size * sizeof(int));

        if (restable->id_results == NULL) {
            printf("Error: realloc() at 'AddResult' [id_results]\n");
            fflush(stdout);
        }
        if (restable->sizes == NULL) {
            printf("Error: realloc() at 'AddResult' [sizes]\n");
            fflush(stdout);
        }
    }

    // update results
    restable->id_results[pos] = results;
    restable->sizes[pos] = sizel;
    restable->size++;
}

void GetResults(ResultsTable *restable, QueryID **results,
                unsigned int *p_num_res, int pos) {
    // point to results and to the size of results
    *results = restable->id_results[pos];
    *p_num_res = restable->sizes[pos];
}

void DestroyResultsTable(ResultsTable *restable) {
    // free all memory
    free(restable->id_results);
    free(restable->sizes);
}