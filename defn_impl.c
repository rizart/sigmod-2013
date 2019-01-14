#include "defn_impl.h"
#include "fnv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--------------------------------Word----------------------------------------//

void CreateWord(const char *word_str, Word *word) {
    // allocate memory for word
    word->word_str = malloc((strlen(word_str) + 1) * sizeof(char));

    if (word->word_str == NULL) {
        printf("Error: malloc() at 'CreateWord'\n");
        fflush(stdout);
    }

    strcpy(word->word_str, word_str);      // copy to struct's string member
    word->length = strlen(word->word_str); // copy length
}

void DestroyWord(Word *word) {
    free(word->word_str);
    word->word_str = NULL;
}

void PrintWord(Word *word) {
    printf("%s", word->word_str);
}

//------------------------------Query-----------------------------------------//

void CreateQuery(QueryID query_id, const char *query_str,
                 enum MatchType match_type, unsigned int match_dist,
                 Query *query) {
    query->id = query_id;           // copy id
    query->match_type = match_type; // copy matching type
    query->match_dist = match_dist; // copy matching distance

    char *parser; // we need to parse the document
    char *saveptr;
    parser = strtok_r((char *)query_str, " ", &saveptr);

    int i = 0;
    while (parser != NULL) {
        query->word[i] = malloc(sizeof(Word)); // allocate space for a Word

        if (query->word[i] == NULL) {
            printf("Error: malloc() at 'CreateQuery'\n");
            fflush(stdout);
        }

        // create word
        CreateWord(parser, query->word[i]);
        parser = strtok_r(NULL, " ", &saveptr);
        i++;
    }

    query->num_of_words = i; // total words of query
}

int GetNumQueryWords(const struct Query *query) {
    return query->num_of_words;
}

const struct Word *getQueryWord(unsigned int word, const Query *query) {
    return query->word[word];
}

void DestroyQuery(Query *query) {
    int i;
    for (i = 0; i != query->num_of_words; i++) {
        DestroyWord(query->word[i]);
        free(query->word[i]);
        query->word[i] = NULL;
    }
}

void PrintQuery(Query *query) {
    int size = query->num_of_words;
    int i;
    printf("================================================\n");
    printf("Query id: %d\n", query->id);
    printf("Query matching type: ");
    if (query->match_type == MT_EXACT_MATCH)
        printf("EXACT MATCH\n");
    if (query->match_type == MT_HAMMING_DIST)
        printf("HAMMING_MATCH\n");
    if (query->match_type == MT_EDIT_DIST)
        printf("EDIT_MATCH\n");
    printf("Query matching distance: %d\n", query->match_dist);
    printf("Query_Words[");
    for (i = 0; i != size; i++) {
        PrintWord(query->word[i]);
        if (i != size - 1)
            printf(",");
    }
    printf("]\n");
    printf("================================================\n");
}

//----------------------------Document----------------------------------------//

void DestroyHashtable(BT_Node **HashTable, int hashprime) {
    int i;

    for (i = 0; i != hashprime; i++) {
        if (HashTable[i] != NULL) {
            dealloc_bt(HashTable[i]);
            free(HashTable[i]);
            HashTable[i] = NULL;
        }
    }
}

void DeDuplicateDocWord(BT_Node **HashTable, Word cword, int *unique_words,
                        Document *document, int bucket) {
    // word was not found in the binary tree thus its unique till now
    if (!search_bt(HashTable[bucket], cword)) {
        // check if there is the same word in the Binary Tree
        HashTable[bucket] = insert_bt(HashTable[bucket], cword);

        document->words[*unique_words] = malloc(sizeof(Word));

        if (document->words[*unique_words] == NULL) {
            printf("Error: malloc() at 'DeDuplicateDocWord'\n");
            fflush(stdout);
        }

        CreateWord(cword.word_str, document->words[*unique_words]);
        (*unique_words)++; // increase amount of unique words
    }
}

void CreateDocument(DocID doc_id, char *doc_str, Document *document) {
    document->id = doc_id; // copy id

    int i, unique_words = 0;
    unsigned int bucket;

    // calculate maximum number of words in the particular document
    int max_doc_words = strlen(doc_str) / MIN_WORD_LENGTH;

    int hashprime = FindNextPrime(max_doc_words);

    document->words = malloc(max_doc_words * sizeof(Word *));

    if (document->words == NULL) {
        printf("Error: malloc() at 'CreateDocument'\n");
        fflush(stdout);
    }

    BT_Node *HashTable[hashprime];
    for (i = 0; i != hashprime; i++)
        HashTable[i] = NULL;

    char *saveptr;
    char *parser; // we need to parse the document
    parser = strtok_r(doc_str, " ", &saveptr);

    while (parser != NULL) {
        Word cword;
        CreateWord(parser, &cword);

        bucket = FNV32(cword.word_str, hashprime);
        DeDuplicateDocWord(HashTable, cword, &unique_words, document, bucket);

        DestroyWord(&cword);
        parser = strtok_r(NULL, " ", &saveptr);
    }

    DestroyHashtable(HashTable, hashprime);
    document->num_of_words = unique_words; // set the new document total words
}

void DestroyDocument(Document *doc) {
    int i;
    int words = GetNumDocumentWords(doc);

    for (i = 0; i != words; i++) {
        DestroyWord(doc->words[i]);
        free(doc->words[i]);
        doc->words[i] = NULL;
    }

    free(doc->words);
    doc->words = NULL;
}

int GetNumDocumentWords(const struct Document *doc) {
    return doc->num_of_words;
}

const struct Word *getDocumentWord(unsigned int word, const Document *doc) {
    return doc->words[word];
}

void PrintDocument(Document *doc) {
    int size = doc->num_of_words;
    int i;
    printf(
        "_________________________________________________________________\n");
    printf("Document id: %d\n", doc->id);
    printf("Document number of words: %d\n", size);
    printf("Document_Words[");
    for (i = 0; i != size; i++) {
        PrintWord(doc->words[i]);
        if (i != size - 1)
            printf(",");
    }
    printf("]\n");
    printf(
        "_________________________________________________________________\n");
}

//---------------------------MatchType----------------------------------------//

int Equal(const Word *w1, const Word *w2) {
    if (w1->length == w2->length)
        if (!memcmp(w1->word_str, w2->word_str, w1->length))
            return 0;
    return 1;
}

int HammingDistance(const Word *w1, const Word *w2) {
    if (w1->length != w2->length)
        return -1;

    int i = 0, matches = 0;
    int length = w1->length;

    while (i != length) {
        // count matches
        matches += (1 >> (w1->word_str[i] - w2->word_str[i]));
        i++;
    }

    return length - matches;
}

int EditDistance(const Word *w1, const Word *w2) {
    int Arr[2][w2->length + 1];

    int i, j, dist;
    int d1, d2, d3;
    int cur = 1;

    for (j = 0; j <= w2->length; j++) // initialize array
        Arr[0][j] = j;

    for (i = 1; i <= w1->length;
         i++) { // memory optimized Needleman-Wunsch algorithm

        Arr[cur][0] = i;

        for (j = 1; j <= w2->length; j++) {
            d1 = Arr[1 - cur][j] + 1; // delete
            d2 = Arr[cur][j - 1] + 1; // insert
            d3 = Arr[1 - cur][j - 1]; // swap

            if (w1->word_str[i - 1] != w2->word_str[j - 1]) // swap + 1
                d3++;

            dist = d1;
            if (d2 < dist)
                dist = d2; // we want the minimum
            if (d3 < dist)
                dist = d3;

            Arr[cur][j] = dist; // set the value to array
        }

        cur = 1 - cur;
    }

    return Arr[1 - cur][w2->length];
}

//--------------------------------Utilities-----------------------------------//

BT_Node *insert_bt(BT_Node *node, Word word) { // Insert in binary tree

    // insert a value to node
    if (node == NULL) {
        node = malloc(sizeof(BT_Node));

        if (node == NULL) {
            printf("Error: malloc() at 'insert_bt'\n");
            fflush(stdout);
        }

        CreateWord(word.word_str, &node->word);
        node->left = NULL;
        node->right = NULL;

    }

    else if (strcmp(word.word_str, node->word.word_str) > 0)
        node->right = insert_bt(node->right, word);
    else if (strcmp(word.word_str, node->word.word_str) < 0)
        node->left = insert_bt(node->left, word);

    return node;
}

int search_bt(BT_Node *node, Word word) { // Search binary tree

    if (node == NULL)
        return 0;

    if (!Equal(&word, &(node->word)))
        return 1;
    else if (strcmp(word.word_str, node->word.word_str) > 0)
        return search_bt(node->right, word);
    else
        return search_bt(node->left, word);
}

void dealloc_bt(BT_Node *node) { // destroy binary tree

    if (node->left != NULL) {
        dealloc_bt(node->left);
        free(node->left);
    }

    if (node->right != NULL) {
        dealloc_bt(node->right);
        free(node->right);
    }

    DestroyWord(&(node->word));
    node = NULL;
}

int FindNextPrime(int number) { // given number find next prime number

    int num, prime, can_div;

    num = number;
    prime = 0;

    while (!prime) {
        prime = 1;
        if (((num != 2) && (num % 2) == 0) || ((num != 3) && ((num % 3) == 0)))
            prime = 0;

        can_div = 5;
        while ((can_div * can_div <= num) && (prime == 1)) {
            if ((num % can_div == 0) || (num % (can_div + 2) == 0))
                prime = 0;

            can_div += 6;
        }
        num++;
    }

    return num - 1;
}
