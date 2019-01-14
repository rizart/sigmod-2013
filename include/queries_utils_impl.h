#ifndef QUERIES_UTILS_IMPL_H
#define QUERIES_UTILS_IMPL_H

#include "queries_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

struct QuerySet { // Struct QuerySet holds a table of queries

    int max_size;
    int size;
    unsigned int total_qset_words; // total words inside the queryset
    Query **table;
};

struct Queries { // Struct Queries holds 4 query sets

    QuerySet edit;    // holds queries with match type edit
    QuerySet hamming; // holds queries with match type hamming
    QuerySet exact;   // holds queries with match type exact
    QuerySet general; // holds all queries
};

struct QueriesBitMap { // Struct QueriesBitMap holds a queries bitmap

    char **bitmap; // represents the queries and the words of them
    int *distance; // represents the queries mathcing distances
    int *counter;  // counter that holds the num of words that have matched so
                   // far
    int *total_qwords; // represents the queries number of words
};

#ifdef __cplusplus
}
#endif

#endif // QUERIES_UTILS_IMPL_H