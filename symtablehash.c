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

/* Define a meaningful constant for the shift amount in the hash function */
#define HASH_SHIFT_AMOUNT 5

/* SymTableNodes consist of keys, values, and the address of the next node. */
struct SymTableNode {
    /* The key */
    char *pcKey;

    /* The value */
    const void *pvValue;

    /* Pointer to the next node in the linked list */
    struct SymTableNode *psNextNode;
};

/* SymTable is the managing structure, containing an array of buckets. */
struct SymTable {
    /* Array of bucket pointers */
    struct SymTableNode *buckets[BUCKET_COUNT];
    
    /* Total number of nodes in the table */
    size_t nodeQuantity;
};

/*--------------------------------------------------------------------*/

/*
 * symtablehash_hashFunction:
 * Calculates the hash code for a given key.
 * Parameters:
 *   pcKey - A pointer to the key (string) to be hashed.
 * Returns:
 *   An unsigned integer representing the hash code of the given key.
 */
static unsigned int symtablehash_hashFunction(const char *pcKey) {
    unsigned int hash = 0U;

    /* Validate that the key is not NULL */
    assert(pcKey != NULL);

    /* Calculate the hash using a left shift and add operation */
    while (*pcKey != '\0') {
        hash = (hash << HASH_SHIFT_AMOUNT) + (unsigned int)(*pcKey++);
    }
    return hash % BUCKET_COUNT;
}

/*--------------------------------------------------------------------*/

/* 
 * SymTable_new:
 * Creates and returns a new, empty SymTable.
 * Returns NULL if memory allocation fails.
 */
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

/* 
 * SymTable_getLength:
 * Returns the number of bindings in the SymTable.
 * Parameters:
 *   oSymTable - A pointer to the SymTable.
 * Returns:
 *   The number of key-value bindings in the table.
 */
size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->nodeQuantity;
}

/*--------------------------------------------------------------------*/

/* 
 * SymTable_free:
 * Frees all memory occupied by the SymTable.
 * Parameters:
 *   oSymTable - A pointer to the SymTable to be freed.
 */
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

/* 
 * SymTable_put:
 * Inserts a new key-value pair into the SymTable.
 * Returns 1 if successful, or 0 if there is a duplicate or memory allocation fails.
 * Parameters:
 *   oSymTable - A pointer to the SymTable.
 *   pcKey - A string representing the key to be inserted.
 *   pvValue - A pointer to the value associated with the key.
 */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    unsigned int index;
    struct SymTableNode *psNewNode, *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);  /* Validate that pvValue is not NULL */

    index = symtablehash_hashFunction(pcKey);

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

/* 
 * SymTable_replace:
 * Replaces the value of an existing key in the SymTable.
 * Returns the old value or NULL if the key is not found.
 * Parameters:
 *   oSymTable - A pointer to the SymTable.
 *   pcKey - A string representing the key whose value needs replacement.
 *   pvValue - A pointer to the new value.
 */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    unsigned int index;
    struct SymTableNode *psCurrentNode;
    void *oldValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);  /* Validate that pvValue is not NULL */

    index = symtablehash_hashFunction(pcKey);

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

/* 
 * SymTable_contains:
 * Checks if the SymTable contains a specified key.
 * Returns 1 if the key is found, 0 otherwise.
 * Parameters:
 *   oSymTable - A pointer to the SymTable.
 *   pcKey - A string representing the key to search for.
 */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    unsigned int index;
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    index = symtablehash_hashFunction(pcKey);

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

/* 
 * SymTable_get:
 * Returns the value associated with the specified key or NULL if not found.
 * Parameters:
 *   oSymTable - A pointer to the SymTable.
 *   pcKey - A string representing the key whose value is to be retrieved.
 */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    unsigned int index;
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    index = symtablehash_hashFunction(pcKey);

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

/* 
 * SymTable_remove:
 * Removes the key-value pair with the specified key from the SymTable.
 * Returns the associated value or NULL if the key is not found.
 * Parameters:
 *   oSymTable - A pointer to the SymTable.
 *   pcKey - A string representing the key to be removed.
 */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    unsigned int index;
    struct SymTableNode *psCurrentNode, *psPrevNode = NULL;
    void *oldValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    index = symtablehash_hashFunction(pcKey);

    psCurrentNode = oSymTable->buckets[index];
}
