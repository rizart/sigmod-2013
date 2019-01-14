#ifndef INDEX_IMPL_H
#define INDEX_IMPL_H

#include "binary_tree.h"
#include "indexes.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------//

struct VP_Tree { // Struct that represents a VP Tree

    VPTnode *root;
};

struct VPTnode { // Struct that represents a VP Tree node

    int median;          // median
    Entry vantage_point; // the vantage point
    VPTnode *left;       // points to left subtree
    VPTnode *right;      // points to right subtree
    EntryList *dataset;  // points to the remaining dataset if node is leaf
    // otherwise needs to be put NULL
};

//----------------------------------------------------------------------------//

struct Edit_Index { // Struct that holds a VP Tree - edit indexing

    VP_Tree vp_tree;
};

struct Hamming_Index { // Struct that holds multiple VP Trees - hamming indexing

    VP_Tree vp_table[MAX_WORD_LENGTH - MIN_WORD_LENGTH + 1];
};

struct Exact_Index { // Struct that holds a hashtable - exact indexing

    int hashtable_size;
    BTree_Node **HashTable;
};

//----------------------------------------------------------------------------//

struct Indexes { // Struct that holds all indexes

    Edit_Index edit;
    Hamming_Index hamming;
    Exact_Index exact;
};

//----------------------------------------------------------------------------//

struct EntryList { // Struct that holds a set of entries

    int size;
    int max_size;
    int current;
    Entry *elist;
};

//----------------------------------------------------------------------------//

#ifdef __cplusplus
}
#endif

#endif /* INDEX_IMPL_H */
