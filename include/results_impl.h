#ifndef RESULTS_UTILS_IMPL_H
#define RESULTS_UTILS_IMPL_H

#include "core.h"
#include "results.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Results_struct {
    int cur;             // points to the next result to be inserted
    QueryID *id_results; // holds some results for a doc
};

struct ResultsTable {
    int max_size;         // max potential size so far
    int size;             // size of table
    QueryID **id_results; // holds all results so far
    int *sizes;           // holds all sizes of results arrays
};

#ifdef __cplusplus
}
#endif

#endif // RESULTS_UTILS_IMPL_H