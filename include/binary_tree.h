#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include "index.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BTree_Node BTree_Node;

// insert entry inside the binary tree - upload payload if needed
BTree_Node *InsertEntryInBTree(BTree_Node *node, Entry entry);

// search binary tree and fill entrylist results with entries matching word
void SearchBTree(BTree_Node *node, Word word, EntryList *results);

// destroy the binary tree
void DestroyBTree(BTree_Node *node);

// extract all binary tree entries into the entries entrylist
void ExtractEntriesFromBTree(BTree_Node *node, EntryList *entries);

#ifdef __cplusplus
}
#endif

#endif // BINARY_TREE_H