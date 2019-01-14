#ifndef INDEXES_H
#define INDEXES_H

#include "index.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------     EDIT  ---------------------------------------//

/*
 * This structure holds the index for the edit distance type entries.
 */
typedef struct Edit_Index Edit_Index;

//----------------------------------------------------------------------------//

// initialize edit index
void InitializeEditIndex(Edit_Index *index);

// destroy edit index
void DestroyEditIndex(Edit_Index *index);

// build edit index given an entrylist and the match type(which should be edit)
void BuildEditIndex(Edit_Index *index, EntryList *entries);

// search the edit index and fill entrylist result with the results
void SearchEditIndex(Edit_Index index, Word word, EntryList *result,
                     EntryList *result_cache);

//----------------------------------------------------------------------------//

//----------------------------   HAMMING    ----------------------------------//

/*
 * This structure holds the index for the hamming distance type entries.
 */
typedef struct Hamming_Index Hamming_Index;

//----------------------------------------------------------------------------//

// initialize hamming index
void InitializeHammingIndex(Hamming_Index *index);

// destroy hamming index
void DestroyHammingIndex(Hamming_Index *index);

// build hamming index given an entrylist
// and the match type(which should be hamming)
void BuildHammingIndex(Hamming_Index *index, EntryList *entries);

// search the hamming index and fill entrylist result with the results
void SearchHammingIndex(Hamming_Index index, Word word, EntryList *result);

//----------------------------------------------------------------------------//

//----------------------------  EXACT    -------------------------------------//

/*
 * This structure holds the index for the exact distance type entries.
 */
typedef struct Exact_Index Exact_Index;

//----------------------------------------------------------------------------//

// initialize exact index
void InitializeExactIndex(Exact_Index *index);

// destroy exact index
void DestroyExactIndex(Exact_Index *index);

// build exact index given an entrylist
void BuildExactIndex(Exact_Index *index, EntryList *entries);

// search the exact index and fill entrylist result with the results
void SearchExactIndex(Exact_Index index, Word word, EntryList *results);

//----------------------------------------------------------------------------//

//-------------------------    INDEXES     -----------------------------------//

/*
 * This structure holds all indexes.
 */
typedef struct Indexes Indexes;

//----------------------------------------------------------------------------//

// initialize all indexes
void InitializeIndexes(Indexes *indexes);

// destroy all indexes
void DestroyIndexes(Indexes *indexes);

// build all indexes with the given querysets(which exist inside 'queries')
void BuildIndexes(Indexes *indexes, Queries *queries);

// search all indexes for the given document id from the DocumentSet and
// return all results in the entrylists depending on the match type
void SearchIndexes(EntryList *hamming_results, EntryList *edit_results,
                   EntryList *exact_results, Document doc, Indexes indexes,
                   int c);

//----------------------------------------------------------------------------//

#ifdef __cplusplus
}
#endif

#endif /* INDEXES_H */