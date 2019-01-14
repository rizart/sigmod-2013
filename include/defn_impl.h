#ifndef DEFN_IMPL_H
#define DEFN_IMPL_H

#include "defn.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Word { // Word struct holds a word

    char *word_str;
    int length;
};

struct Query { // Query struct holds a query

    QueryID id;                  // query id
    Word *word[MAX_QUERY_WORDS]; // query words
    enum MatchType match_type;   // query matching type
    unsigned int match_dist;     // query matching distance
    unsigned int num_of_words;   // number of query words
};

struct Document { // Document struct holds a document

    DocID id;                  // document id
    Word **words;              // document deduplicated words - no duplicates
    unsigned int num_of_words; // number of document words
};

struct BT_Node { // binary tree node tha holds a word

    Word word;
    struct BT_Node *right;
    struct BT_Node *left;
};

#ifdef __cplusplus
}
#endif

#endif /* DEFN_IMPL_H */
