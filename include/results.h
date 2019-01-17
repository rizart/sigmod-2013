#ifndef RESULTS_H
#define RESULTS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ResultsTable ResultsTable;

/*
 * This structure holds a docs results
 */
typedef struct Results_struct Results_struct;

// create the results table
void CreateResultsTable(ResultsTable *restable);

// add result to the result table
void AddResult(ResultsTable *restable, QueryID *results, int size, int pos);

// get results from result table
void GetResults(ResultsTable *restable, QueryID **results,
                unsigned int *p_num_res, int pos);

// destroy result table
void DestroyResultsTable(ResultsTable *restable);

#ifdef __cplusplus
}
#endif

#endif // RESULTS_H