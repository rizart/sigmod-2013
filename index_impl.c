#include "index_impl.h"
#include "core.h"
#include "defn_impl.h"
#include "fnv.h"
#include "index.h"
#include "queries_utils_impl.h"
#include "vp_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Entry

enum ErrorCode CreateEntry(const Word *word, Entry *entry) {
    entry->word = (Word *)word; // pointer to the real word
    entry->payload.size = 0;
    entry->payload.max_size = 512; // max potential size
    entry->payload.table =
        malloc(entry->payload.max_size * sizeof(Payload_Member));

    if (entry->payload.table == NULL) {
        printf("Error: malloc() at 'CreateEntry'\n");
        fflush(stdout);
    }

    return EC_SUCCESS;
}

enum ErrorCode DestroyEntry(Entry *entry) {
    entry->word = NULL;
    free(entry->payload.table);
    return EC_SUCCESS;
}

void PrintEntry(Entry *entry) {
    PrintWord(entry->word);
}

void UpdatePayload(Entry *entry, int query_id, int position) {
    int size = entry->payload.size;

    if (size == entry->payload.max_size) { // payload is full
        entry->payload.max_size *= 2;
        entry->payload.table =
            realloc(entry->payload.table,
                    entry->payload.max_size * sizeof(Payload_Member));
        if (entry->payload.table == NULL) {
            printf("Error: realloc() at 'UpdatePayload'\n");
            fflush(stdout);
        }
    }

    // update payload
    entry->payload.size++;
    entry->payload.table[size].qid = query_id;
    entry->payload.table[size].position = position;
}

void CopyPayload(Entry *dest, const Entry *source) {
    int payloadsize = source->payload.size;
    int payloadmaxsize = source->payload.max_size;
    int i;

    if (dest->payload.max_size != payloadmaxsize) { // payload is full
        dest->payload.table = realloc(dest->payload.table,
                                      payloadmaxsize * sizeof(Payload_Member));
        if (dest->payload.table == NULL) {
            printf("Error: realloc() at 'CopyPayload'\n");
            fflush(stdout);
        }
    }

    dest->payload.max_size = payloadmaxsize;
    dest->payload.size = payloadsize;
    // copy payload
    for (i = 0; i != payloadsize; i++) {
        dest->payload.table[i].qid = source->payload.table[i].qid;
        dest->payload.table[i].position = source->payload.table[i].position;
    }
}

void CopyEntry(Entry *dest, const Entry *source) {
    CreateEntry(source->word, dest);
    CopyPayload(dest, source);
}

// EntryList

enum ErrorCode CreateEntryList(EntryList *entryList) {
    entryList->size = 0;
    entryList->current = 0;
    entryList->max_size = 1024;
    entryList->elist = malloc(entryList->max_size * sizeof(Entry));

    if (entryList->elist == NULL) {
        printf("Error: malloc() at 'CreateEntryList'\n");
        fflush(stdout);
    }

    return EC_SUCCESS;
}

enum ErrorCode DestroyEntryList(EntryList *entryList) {
    int size = entryList->size;
    int i;

    for (i = 0; i != size; i++)
        DestroyEntry(&(entryList->elist[i]));

    free(entryList->elist);
    entryList->elist = NULL;
    return EC_SUCCESS;
}

enum ErrorCode AddEntry(EntryList *entryList, const Entry *entry) {
    int size = entryList->size;
    int max_size = entryList->max_size;

    if (size == max_size) { // we need reallocation

        entryList->max_size *= 2;
        entryList->elist =
            realloc(entryList->elist, entryList->max_size * sizeof(Entry));
        if (entryList->elist == NULL) {
            printf("Error: realloc() at 'AddEntry'\n");
            fflush(stdout);
        }
    }
    CopyEntry(&entryList->elist[size], entry);
    entryList->size++;
    return EC_SUCCESS;
}

void CopyEntryList(EntryList *dest, EntryList *source, int emptydest) {
    if (emptydest)
        CreateEntryList(dest);

    int i, size = source->size;

    for (i = 0; i != size; i++)
        AddEntry(dest, &source->elist[i]);
}

Entry *GetFirst(EntryList *entryList) {
    entryList->current = 1;
    return &(entryList->elist[0]);
}

Entry *GetNext(EntryList *entryList) {
    int cur = entryList->current;
    entryList->current++;
    if (entryList->current == entryList->size) // rotate
        entryList->current = 0;

    return &(entryList->elist[cur]);
}

unsigned int GetNumberOfEntries(const EntryList *entryList) {
    return entryList->size;
}

void PrintEntryList(const EntryList *entryList) {
    int size = entryList->size;
    int i;
    printf("EntryList_Words[");
    for (i = 0; i != size; i++) {
        PrintEntry(&entryList->elist[i]);
        if (i != size - 1)
            printf(",");
    }
    printf("]\n");
}

void PrintEntrylistPayloads(EntryList *entries) {
    int size = entries->size;
    int i, j;

    for (i = 0; i != size; i++) {
        printf("---------------------------------------------\n");
        printf("Entry with word %s\n", entries->elist[i].word->word_str);
        int paysize = entries->elist[i].payload.size;
        for (j = 0; j != paysize; j++) {
            printf("[Query id: %d | ", entries->elist[i].payload.table[j].qid);
            printf("Position : %d]\n",
                   entries->elist[i].payload.table[j].position);
        }
    }
    printf("---------------------------------------------\n");
}
