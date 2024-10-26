/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
/* Enhanced Version: Implements a hash table with dynamic resizing    */
/* Author: Ephraim Meles                                              */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/* Define initial number of buckets and resizing threshold. */
#define INITIAL_BUCKET_COUNT 509
#define LOAD_FACTOR_THRESHOLD 0.75

/* An array of prime numbers for resizing the hash table. */
static const size_t primes[] = {
    509, 1021, 2039, 4093, 8191, 16381, 32771, 65537, 131071, 262147
};

/* 
 * PRIME_COUNT:
 * The total number of prime numbers in the `primes` array.
 */
static const size_t PRIME_COUNT = sizeof(primes) / sizeof(primes[0]);

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
    struct SymTableNode **buckets;

    /* Current number of buckets */
    size_t bucketCount;

    /* Total number of nodes in the table */
    size_t nodeQuantity;

    /* Current index in the primes array for resizing */
    size_t currentPrimeIndex;
};

/*--------------------------------------------------------------------*/

/*
 * symtablehash_hashFunction:
 * Calculates the hash code for a given key.
 * Parameters:
 *   pcKey - A pointer to the key (string) to be hashed.
 *   bucketCount - The current number of buckets in the hash table.
 * Returns:
 *   An unsigned integer representing the hash code of the given key.
 */
static unsigned int symtablehash_hashFunction(const char *pcKey, size_t bucketCount) {
    unsigned int hash = 0U;

    /* Validate that the key is not NULL */
    assert(pcKey != NULL);

    /* Calculate the hash using a left shift and add operation */
    while (*pcKey != '\0') {
        hash = (hash << HASH_SHIFT_AMOUNT) + (unsigned int)(*pcKey++);
    }
    return hash % bucketCount;
}

/*--------------------------------------------------------------------*/

/* 
 * SymTable_new:
 * Creates and returns a new, empty SymTable.
 * Returns NULL if memory allocation fails.
 */
SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;
    
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable == NULL) return NULL;

    oSymTable->currentPrimeIndex = 0;
    oSymTable->bucketCount = primes[oSymTable->currentPrimeIndex];
    oSymTable->nodeQuantity = 0;
    oSymTable->buckets = (struct SymTableNode**)calloc(oSymTable->bucketCount, sizeof(struct SymTableNode*));
    
    if (oSymTable->buckets == NULL) {
        free(oSymTable);
        return NULL;
    }

    return oSymTable;
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
    size_t i;  /* Declare variable at the top for C90 compliance */

    assert(oSymTable != NULL);

    i = 0;
    while (i < oSymTable->bucketCount) {
        psCurrentNode = oSymTable->buckets[i];
        while (psCurrentNode != NULL) {
            psNextNode = psCurrentNode->psNextNode;
            free((char*)psCurrentNode->pcKey);
            free(psCurrentNode);
            psCurrentNode = psNextNode;
        }
        i++;
    }
    free(oSymTable->buckets);
    free(oSymTable);
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
 * resizeHashTable:
 * Resizes the hash table when the load factor exceeds the threshold.
 * Parameters:
 *   oSymTable - A pointer to the SymTable to be resized.
 */
static void symtablehash_resizeHashTable(SymTable_T oSymTable) {
    size_t newPrimeIndex;
    size_t newBucketCount;
    struct SymTableNode **newBuckets;
    size_t i;

    newPrimeIndex = oSymTable->currentPrimeIndex + 1;
    if (newPrimeIndex >= PRIME_COUNT) return;  /* No more resizing */

    newBucketCount = primes[newPrimeIndex];
    newBuckets = (struct SymTableNode**)calloc(newBucketCount, sizeof(struct SymTableNode*));
    if (newBuckets == NULL) return;  /* Allocation failed, skip resizing */

    /* Rehash all existing nodes into the new buckets */
    i = 0;
    while (i < oSymTable->bucketCount) {
        struct SymTableNode *psCurrentNode = oSymTable->buckets[i];
        while (psCurrentNode != NULL) {
            struct SymTableNode *psNextNode = psCurrentNode->psNextNode;
            unsigned int newIndex = symtablehash_hashFunction(psCurrentNode->pcKey, newBucketCount);

            /* Insert node into the new bucket array */
            psCurrentNode->psNextNode = newBuckets[newIndex];
            newBuckets[newIndex] = psCurrentNode;

            psCurrentNode = psNextNode;
        }
        i++;
    }

    /* Replace the old buckets with the new ones */
    free(oSymTable->buckets);
    oSymTable->buckets = newBuckets;
    oSymTable->bucketCount = newBucketCount;
    oSymTable->currentPrimeIndex = newPrimeIndex;
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

    /* Check if resizing is needed */
    if ((double)oSymTable->nodeQuantity / oSymTable->bucketCount > LOAD_FACTOR_THRESHOLD) {
        resizeHashTable(oSymTable);
    }

    index = symtablehash_hashFunction(pcKey, oSymTable->bucketCount);

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

    index = symtablehash_hashFunction(pcKey, oSymTable->bucketCount);

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

    index = symtablehash_hashFunction(pcKey, oSymTable->bucketCount);

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

    index = symtablehash_hashFunction(pcKey, oSymTable->bucketCount);

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

    index = symtablehash_hashFunction(pcKey, oSymTable->bucketCount);

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

/* 
 * SymTable_map:
 * Applies the given function *pfApply to each key-value pair in the SymTable.
 * Parameters:
 *   oSymTable - A pointer to the SymTable.
 *   pfApply - A pointer to a function that takes three parameters:
 *     - The key (const char *)
 *     - The value (void *)
 *     - An extra parameter provided by the caller (void *)
 *   pvExtra - A pointer to the extra parameter that will be passed to *pfApply.
 */
void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
                  const void *pvExtra) {
    struct SymTableNode *psCurrentNode;
    size_t i;

    assert(pvExtra != NULL);
    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    i = 0;
    while (i < oSymTable->bucketCount) {
        psCurrentNode = oSymTable->buckets[i];
        while (psCurrentNode != NULL) {
            (*pfApply)(psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, (void*)pvExtra);
            psCurrentNode = psCurrentNode->psNextNode;
        }
        i++;
    }
}
