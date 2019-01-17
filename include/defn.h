#ifndef DEFN_H
#define DEFN_H

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

// Structures

/*
 * This structure holds information about a particlar word.
 * You have to define it.
 */
typedef struct Word Word;

/*
 * This structure holds information about a particlar query.
 * You have to define it.
 */
typedef struct Query Query;

/*
 * This structure holds information about a particlar document.
 * You have to define it.
 */
typedef struct Document Document;

/*
 * This structure holds information about a particlar Binary Tree node.
 */
typedef struct BT_Node BT_Node;

// Word

/*
 * Create the given word from the string specified.
 */
void CreateWord(const char *word_str, Word *word);

/*
 * Destroy the given word
 */
void DestroyWord(Word *word);

/*
 * Print the given word
 */
void PrintWord(Word *word);

// Query

/*
 * Create the given query from the parameters specified.
 *
 * @param[in] query_id
 *   The integral ID of the query. This function will not be called twice
 *   with the same query ID.
 *
 * @param[in] query_str
 *   A null-terminated string representing the query. It consists of
 *   a space separated set of words. The length of any word will be at
 *	least MIN_WORD_LENGTH characters and at most MAX_WORD_LENGTH
 *	characters. The number of words in a query will not exceed
 *	MAX_QUERY_WORDS words.
 *   "query_str" contains at least one non-space character.
 *   "query_str" contains only lower case letters from 'a' to 'z'
 *   and space characters.
 *
 * @param[in] match_type
 *   The type of mechanism used to consider a query as a
 *   match to any document, as specified in MatchType enumeration.
 *
 * @param[in] match_dist
 *   The hamming or edit distance (according to "match_type")
 *   threshold used as explained in MatchType enumeration.
 *   This parameter must be equal 0 for exact matching. The possible
 *   values of this parameter are 0,1,2,3.
 *   A query matches a document if and only if: for each word in
 *	the query, there exist a word in the document that matches it
 *	under the "match_type" and "match_dist" constraints. Note that
 *	the "match_dist" constraint is applied independently for each
 *	word in the query.
 */
void CreateQuery(QueryID query_id, const char *query_str,
                 enum MatchType match_type, unsigned int match_dist,
                 Query *query);

/*
 * Destroy the given query
 */
void DestroyQuery(Query *query);

/*
 * Return the number of words of the given query.
 */
int GetNumQueryWords(const Query *query);

/*
 * Get a particular word from the given query.
 *
 * Example:
 *   Query = "hello world"
 *   Word 0 = "hello"
 *   Word 1 = "world"
 */
const struct Word *getQueryWord(unsigned int word, const Query *query);

/*
 * Print the contents of the given query.
 */
void PrintQuery(Query *query);

// Document

/*
 * Create the given document from the parameters specified.
 *
 * @param[in] doc_id
 *   The integral ID of the document. This function will not be called twice
 *   with the same document ID.
 *
 * @param[in] doc_str
 *   A null-terminated string representing the document. It consists of
 *   a space separated set of words. The length of any word
 *   will be at least MIN_WORD_LENGTH characters and at most
 *	MAX_WORD_LENGTH characters. The length of any document will not
 *   exceed MAX_DOC_LENGTH characters.
 *   "doc_str" contains at least one non-space character.
 *   "doc_str" contains only lower case letters from 'a' to 'z'
 *   and space characters.
 *
 *   @return ErrorCode
 *   - \ref EC_SUCCESS
 *          if the document was added successfully
 */
void CreateDocument(DocID doc_id, char *doc_str, Document *doc);

/*
 * Destroy the given document
 */
void DestroyDocument(Document *doc);

/*
 * Return the number of words of the given document.
 */
int GetNumDocumentWords(const Document *doc);

/*
 * Get a particular word from the given document.
 *
 * Example:
 *   Doc    = "hello world"
 *   Word 0 = "hello"
 *   Word 1 = "world"
 */
const struct Word *getDocumentWord(unsigned int word, const Document *doc);

/*
 * Print the contents of the given document.
 */
void PrintDocument(Document *doc);

// Distance

/*
 * Return 0 if the two words are equal, else return 1
 */
int Equal(const Word *w1, const Word *w2);

/*
 * Return the hamming distance of the two words. If the length of the two words
 * is different, return -1
 */
int HammingDistance(const Word *w1, const Word *w2);

/*
 * Return the edit distance of the two words.
 */
int EditDistance(const Word *w1, const Word *w2);

// Utilities

/*
 * Method to remove the duplicate words from the document specified.
 */
void RemoveDuplicates(Document *doc);

/*
 * Integrated part of RemoveDuplicates
 * It inserts a word to a Binary Tree depending on the returned value from
 * search
 */
BT_Node *insert_bt(BT_Node *node, Word word);

/*
 * Integrated part of RemoveDuplicates
 * It searches a Binary Tree and finds if it has to insert the word.
 * Returns 0 if it finds the word in the Binary Tree, otherwise it returns 1
 */
int search_bt(BT_Node *node, Word word);

/*
 * Integrated part of RemoveDuplicates
 * It destroys a Binary Tree.
 */
void dealloc_bt(BT_Node *node);

/*
 * Integrated part of RemoveDuplicates
 * Given a number returns the next prime number
 */
int FindNextPrime(int number);

#ifdef __cplusplus
}
#endif

#endif /* DEFN_H */
