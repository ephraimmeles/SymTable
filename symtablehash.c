/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
/* Author: Ephraim Meles                                              */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/* Define the number of buckets for the hash table. */
#define BUCKET_COUNT 509  /* A prime number to reduce collisions */

/* SymTableNodes consist of keys, values, and the address of the next
   node in the linked list. */
struct SymTableNode {
    char *pcKey;
    const void *pvValue;
    struct SymTableNode *psNextNode;
};

/* SymTable is the managing structure, containing an array of buckets. */
struct SymTable {
    struct SymTableNode *buckets[BUCKET_COUNT];
    size_t nodeQuantity;  /* Total number of nodes in the table */
};

/*--------------------------------------------------------------------*/

/* A simple hash function to calculate the hash code of a key. */
static unsigned int hashFunction(const char *pcKey) {
    unsigned int hash = 0U;
    while (*pcKey != '\0') {
        hash = (hash << 5) + *pcKey++;
    }
    return hash % BUCKET_COUNT;
}

/*--------------------------------------------------------------------*/

/* Returns a new, empty SymTable. Returns NULL if memory allocation fails. */
SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;
    int i;

    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable == NULL) return NULL;

    i = 0;
    while (i < BUCKET_COUNT) {
        oSymTable->buckets[i] = NULL;
        i++;
    }
    
    oSymTable->nodeQuantity = 0;
    return oSymTable;
}

/*--------------------------------------------------------------------*/

/* Returns the number of bindings in oSymTable. */
size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->nodeQuantity;
}

/*--------------------------------------------------------------------*/

/* Frees all memory occupied by oSymTable. */
void SymTable_free(SymTable_T oSymTable) {
    struct SymTableNode *psCurrentNode, *psNextNode;
    int i;

    assert(oSymTable != NULL);

    i = 0;
    while (i < BUCKET_COUNT) {
        psCurrentNode = oSymTable->buckets[i];
        while (psCurrentNode != NULL) {
            psNextNode = psCurrentNode->psNextNode;
            free((char*)psCurrentNode->pcKey);
            free(psCurrentNode);
            psCurrentNode = psNextNode;
        }
        i++;
    }
    free(oSymTable);
}

/*--------------------------------------------------------------------*/

/* Inserts a new key-value pair into oSymTable. Returns 1 if successful,
   or 0 if there is a duplicate or memory allocation fails. */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    unsigned int index;
    struct SymTableNode *psNewNode, *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    index = hashFunction(pcKey);

    /* Check for duplicate keys */
    psCurrentNode = oSymTable->buckets[index];
    while (psCurrentNode != NULL) {
        if (strcmp(pcKey, psCurrentNode->pcKey) == 0) {
            return 0;
        }
        psCurrentNode = psCurrentNode->psNextNode;
    }

    /* Create a new node for the key-value pair */
    psNewNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));
    if (psNewNode == NULL) return 0;

    psNewNode->pcKey = malloc(strlen(pcKey) + 1);
    if (psNewNode->pcKey == NULL) {
        free(psNewNode);
        return 0;
    }
    strcpy(psNewNode->pcKey, pcKey);
    psNewNode->pvValue = pvValue;

    /* Insert the new node into the hash table */
    psNewNode->psNextNode = oSymTable->buckets[index];
    oSymTable->buckets[index] = psNewNode;
    oSymTable->nodeQuantity++;
    return 1;
}

/*--------------------------------------------------------------------*/

/* Replaces the value of an existing key. Returns the old value or NULL if not found. */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    unsigned int index;
    struct SymTableNode *psCurrentNode;
    void *oldValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    index = hashFunction(pcKey);

    psCurrentNode = oSymTable->buckets[index];
    while (psCurrentNode != NULL) {
        if (strcmp(pcKey, psCurrentNode->pcKey) == 0) {
            oldValue = (void*)psCurrentNode->pvValue;
            psCurrentNode->pvValue = pvValue;
            return oldValue;
        }
        psCurrentNode = psCurrentNode->psNextNode;
    }
    return NULL;
}

/*--------------------------------------------------------------------*/

/* Checks if oSymTable contains the specified key. Returns 1 if found, 0 otherwise. */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    unsigned int index;
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    index = hashFunction(pcKey);

    psCurrentNode = oSymTable->buckets[index];
    while (psCurrentNode != NULL) {
        if (strcmp(pcKey, psCurrentNode->pcKey) == 0) {
            return 1;
        }
        psCurrentNode = psCurrentNode->psNextNode;
    }
    return 0;
}

/*--------------------------------------------------------------------*/

/* Returns the value associated with the specified key or NULL if not found. */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    unsigned int index;
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    index = hashFunction(pcKey);

    psCurrentNode = oSymTable->buckets[index];
    while (psCurrentNode != NULL) {
        if (strcmp(pcKey, psCurrentNode->pcKey) == 0) {
            return (void*)psCurrentNode->pvValue;
        }
        psCurrentNode = psCurrentNode->psNextNode;
    }
    return NULL;
}

/*--------------------------------------------------------------------*/

/* Removes the key-value pair with the specified key from oSymTable.
   Returns the associated value or NULL if the key is not found. */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    unsigned int index;
    struct SymTableNode *psCurrentNode, *psPrevNode = NULL;
    void *oldValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    index = hashFunction(pcKey);

    psCurrentNode = oSymTable->buckets[index];
    while (psCurrentNode != NULL) {
        if (strcmp(pcKey, psCurrentNode->pcKey) == 0) {
            oldValue = (void*)psCurrentNode->pvValue;

            /* Adjust pointers to remove the node */
            if (psPrevNode == NULL) {
                oSymTable->buckets[index] = psCurrentNode->psNextNode;
            } else {
                psPrevNode->psNextNode = psCurrentNode->psNextNode;
            }
            
            free((char*)psCurrentNode->pcKey);
            free(psCurrentNode);
            oSymTable->nodeQuantity--;
            return oldValue;
        }
        psPrevNode = psCurrentNode;
        psCurrentNode = psCurrentNode->psNextNode;
    }
    return NULL;
}

/*--------------------------------------------------------------------*/

/* Applies the function *pfApply to each binding in the SymTable. */
void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
                  const void *pvExtra) {
    struct SymTableNode *psCurrentNode;
    int i;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    i = 0;
    while (i < BUCKET_COUNT) {
        psCurrentNode = oSymTable->buckets[i];
        while (psCurrentNode != NULL) {
            (*pfApply)(psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, (void*)pvExtra);
            psCurrentNode = psCurrentNode->psNextNode;
        }
        i++;
    }
}
