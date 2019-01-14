#include "vp_tree.h"
#include "core.h"
#include "defn_impl.h"
#include "index.h"
#include "vp_utilities.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_OF_ENTRIES_MAX_DEPTH 0

//------------------------VP UTILITIES----------------------------------------//

// Find the vantage point and the median
int Find_Vp_Median(EntryList *vp_candidates, EntryList *subset_entries,
                   int *median, enum MatchType type)

{
    int max, dist;
    int total_vp_candidates = GetNumberOfEntries(vp_candidates);
    int subset_size = GetNumberOfEntries(subset_entries);
    int i, j;

    float standard_dev[total_vp_candidates];
    int vp_cand_edit[total_vp_candidates][subset_size];

    for (i = 0; i != total_vp_candidates; i++) {
        standard_dev[i] = 0.0;
        for (j = 0; j != subset_size; j++) {
            vp_cand_edit[i][j] = 0;
        }
    }

    Word *vp_word = GetFirst(vp_candidates)->word;
    for (i = 0; i != total_vp_candidates; i++) {
        Word *sb_word = GetFirst(subset_entries)->word;
        for (j = 0; j != subset_size; j++) {
            if (type == MT_EDIT_DIST)
                dist = EditDistance(vp_word, sb_word);
            else
                dist = HammingDistance(vp_word, sb_word);

            vp_cand_edit[i][j] = dist;
            sb_word = GetNext(subset_entries)->word;
        }
        standard_dev[i] =
            standard_deviation(vp_cand_edit[i], total_vp_candidates);
        vp_word = GetNext(vp_candidates)->word;
    }

    // find the maximum standard deviation - thus the vp
    max = FindMax(standard_dev, total_vp_candidates);

    // find median
    radixsort(vp_cand_edit[max], subset_size); // sort the distances

    if (!subset_size % 2) // take median
        *median = (vp_cand_edit[max][subset_size / 2] +
                   vp_cand_edit[max][(subset_size / 2) + 1]) /
                  2;
    else
        *median = vp_cand_edit[max][subset_size / 2];

    return max;
}

//----------------------------------------------------------------------------//

// Generate two new sets of entries for left and right child respectfully
void GenerateNewSets(EntryList *left_subtree, EntryList *right_subtree,
                     EntryList *entries, VPTnode *node, int vp, int median,
                     enum MatchType type) {
    int i;
    int total_entries = GetNumberOfEntries(entries);
    int dist;

    for (i = 0; i != total_entries; i++) {
        if (i != vp) {
            if (type == MT_EDIT_DIST)
                dist = EditDistance(entries->elist[vp].word,
                                    entries->elist[i].word);
            else
                dist = HammingDistance(entries->elist[vp].word,
                                       entries->elist[i].word);

            if (dist < median)
                AddEntry(left_subtree, &entries->elist[i]);
            else
                AddEntry(right_subtree, &entries->elist[i]);
        }
    }
}

//--------------------------VP TREE-------------------------------------------//

// Initialize vp tree
void InitializeVpTree(VP_Tree *vp_tree) {
    vp_tree->root = malloc(sizeof(VPTnode));

    if (vp_tree->root == NULL) {
        printf("Error: malloc() at 'InitializeVpTree'\n");
        fflush(stdout);
    }

    vp_tree->root->vantage_point.payload.table = NULL;
    vp_tree->root->left = NULL;
    vp_tree->root->right = NULL;
    vp_tree->root->dataset = NULL;
}

// Build vp tree recursively
void BuildVpTree(VPTnode *node, EntryList *entries, enum MatchType type) {
    int total_entries = GetNumberOfEntries(entries);
    int subset_size = floor((total_entries / 4));
    int total_vp_candidates = 5;
    int vp = -1, median, i;

    if (total_entries <= 10) { // partition sizes
        if (total_entries != 1) {
            subset_size = floor(total_entries / 2);
            total_vp_candidates = ceil(total_entries / 2);
        } else {
            subset_size = 0;
            total_vp_candidates = 1;
        }
    }

    // take the first total_vp_candidates of the entries
    EntryList vp_candidates;
    CreateEntryList(&vp_candidates);
    for (i = 0; i != total_vp_candidates; i++)
        AddEntry(&vp_candidates, &entries->elist[i]);
    //-------------------------------------------------

    // take the next subset_size of the entries
    EntryList subset_entries;
    CreateEntryList(&subset_entries);
    for (i = 0; i != subset_size; i++)
        AddEntry(&subset_entries, &entries->elist[total_vp_candidates + i]);
    //------------------------------------------------

    //"vp" will represent the position of the vp point
    if (subset_size)
        vp = Find_Vp_Median(&vp_candidates, &subset_entries, &median, type);

    DestroyEntryList(&vp_candidates);
    DestroyEntryList(&subset_entries);

    if (vp == -1) { // last entry thus the vp

        node->dataset = NULL;
        node->median = -1; // theres no median
        node->right = NULL;
        node->left = NULL;

        CopyEntry(&node->vantage_point, &entries->elist[0]);
        return;

    } else { // not last entry

        node->dataset = NULL;
        node->median = median;

        CopyEntry(&node->vantage_point, &entries->elist[vp]);

        // max depth reached so this will become a leaf node
        if (total_entries <= NUM_OF_ENTRIES_MAX_DEPTH + 1) {
            node->dataset = malloc(sizeof(EntryList));

            if (node->dataset == NULL) {
                printf("Error: malloc() at 'BuildVpTree' [node->dataset]\n");
                fflush(stdout);
            }

            CreateEntryList(node->dataset);
            for (i = 0; i != total_entries; i++) {
                if (i != vp)
                    AddEntry(node->dataset, &entries->elist[i]);
            }
            node->median = -1; // theres no median
            node->right = NULL;
            node->left = NULL;
            return;
        }
    }

    EntryList left_subtree;
    EntryList right_subtree;

    CreateEntryList(&left_subtree);
    CreateEntryList(&right_subtree);

    // create the sublists that will exapnd the tree
    GenerateNewSets(&left_subtree, &right_subtree, entries, node, vp, median,
                    type);

    //--------------------------------------------------------------------------

    if (left_subtree.size) { // expand the left child

        node->left = malloc(sizeof(VPTnode));

        if (node->left == NULL) {
            printf("Error: malloc() at 'BuildVpTree [node->left]'\n");
            fflush(stdout);
        }

        BuildVpTree(node->left, &left_subtree, type);
    } else
        node->left = NULL;

    if (right_subtree.size) { // expand the right child

        node->right = malloc(sizeof(VPTnode));

        if (node->right == NULL) {
            printf("Error: malloc() at 'BuildVpTree [node->right]'\n");
            fflush(stdout);
        }

        BuildVpTree(node->right, &right_subtree, type);
    } else
        node->right = NULL;

    DestroyEntryList(&left_subtree);
    DestroyEntryList(&right_subtree);
}

// Search vp tree recursively
void SearchVpTree(const Word *word, int threshold, const VPTnode *node,
                  EntryList *result, EntryList *result_cache,
                  enum MatchType type) {
    int dist;

    if (node->vantage_point.payload.table == NULL)
        return;

    if (type == MT_EDIT_DIST)
        dist = EditDistance(word, node->vantage_point.word);
    else
        dist = HammingDistance(word, node->vantage_point.word);

    if (dist <= threshold) {
        AddEntry(result, &(node->vantage_point));
        if (result_cache != NULL)
            AddEntry(result_cache, &(node->vantage_point));
    }

    if (node->dataset !=
        NULL) { // this is leaf node with some entries in dataset
        int tempdist, i;
        int size = node->dataset->size;
        for (i = 0; i != size; i++) {
            if (type == MT_EDIT_DIST)
                tempdist = EditDistance(word, node->dataset->elist[i].word);
            else
                tempdist = HammingDistance(word, node->dataset->elist[i].word);

            if (tempdist <= threshold) {
                AddEntry(result, &(node->dataset->elist[i]));
                AddEntry(result_cache, &(node->dataset->elist[i]));
            }
        }
        return;
    }

    if ((dist <= threshold + node->median) && node->left != NULL) {
        SearchVpTree(word, threshold, node->left, result, result_cache, type);
    }

    if ((node->median <= threshold + dist) && node->right != NULL) {
        SearchVpTree(word, threshold, node->right, result, result_cache, type);
    }
}

// Destroy vp tree recursively
void DestroyVpTree(VPTnode *node) {
    if (node->left != NULL)
        DestroyVpTree(node->left);
    if (node->right != NULL)
        DestroyVpTree(node->right);

    if (node->dataset != NULL) {
        DestroyEntryList(node->dataset);
        free(node->dataset);
        node->dataset = NULL;
    }

    if (node->vantage_point.payload.table != NULL)
        DestroyEntry(&node->vantage_point);

    free(node);
    node = NULL;
}
