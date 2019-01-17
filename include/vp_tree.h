#ifndef VP_TREE_H
#define VP_TREE_H

#include "index_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

// VP UTILITIES

/*
 *Finds the best vp from a set of vp candidates
 */
int Find_Vp_Median(EntryList *candidate_vp, EntryList *subset, int *median,
                   enum MatchType type);

/*
 * Generates new sets
 */
void GenerateNewSets(EntryList *left_subtree, EntryList *right_subtree,
                     EntryList *entries, VPTnode *node, int vp, int median,
                     enum MatchType type);

// VP TREE

/*
 * Initialize a vp tree
 */
void InitializeVpTree(VP_Tree *vp_tree);

/*
 * Builds a vp tree recursively
 */
void BuildVpTree(VPTnode *node, EntryList *entries, enum MatchType type);

/*
 * Search a vp tree recursively
 */
void SearchVpTree(const Word *word, int threshold, const VPTnode *node,
                  EntryList *result, EntryList *result_cache,
                  enum MatchType type);

/*
 * Destroys a vp tree recursively
 */
void DestroyVpTree(VPTnode *node);

#ifdef __cplusplus
}
#endif

#endif // VP_TREE_H