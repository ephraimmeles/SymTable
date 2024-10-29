/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
/* Enhanced Version: Implements a hash table with dynamic resizing    */
/* Author: Ephraim Meles                                              */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/*
 * INITIAL_BUCKET_COUNT: Sets the initial number of buckets in the hash table.
 * Chose 509 as a prime number to help evenly distribute keys across buckets.
 */
#define INITIAL_BUCKET_COUNT 509
/*
 * LOAD_FACTOR_THRESHOLD: The maximum load factor (number of entries per bucket)
 * allowed before the table resizes. 0.75 means the table resizes once it’s 75% full.
 */
#define LOAD_FACTOR_THRESHOLD 0.75

/*
 * primes: Array of prime numbers used for resizing the table to reduce collisions.
 * Each prime value is selected to increase bucket count when resizing the table.
 */
static const size_t primes[] = {
    509, 1021, 2039, 4093, 8191, 16381, 32771, 65537, 131071, 262147
};

/*
 * PRIME_COUNT: The number of prime numbers in the `primes` array.
 * Used to check if resizing is possible or if the table has reached its max size.
 */
static const size_t PRIME_COUNT = sizeof(primes) / sizeof(primes[0]);

/*
 * HASH_SHIFT_AMOUNT: Defines the left shift amount used in the hash function.
 * This helps spread the bits of each character in the key, improving the hash.
 */
#define HASH_SHIFT_AMOUNT 5

/*
 * SymTableNode: Represents a single entry in the hash table. Each node stores
 * a key-value pair and a pointer to the next node in its bucket.
 */
struct SymTableNode {
    /* The key */
    char *pcKey;

    /* The value */
    const void *pvValue;

    /* Pointer to the next node in the linked list */
    struct SymTableNode *psNextNode;
};

/*
 * SymTable: Main structure for managing the symbol table. 
 * Contains an array of buckets, a count of nodes, the current bucket count, 
 * and the index of the current resizing prime.
 */
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

/*
 * Hashes a key to find its bucket index in the table.
 * Arguments:
 *   - `pcKey`: the string key to hash
 *   - `bucketCount`: total number of buckets in the hash table
 * The function calculates a hash by shifting and adding each character in `pcKey`.
 * Returns an index (unsigned int) for storing the key-value pair.
 */
static unsigned int symtablehash_hashFunction(const char *pcKey, size_t bucketCount) {
    unsigned int hash = 0U;

    /* Validate that the key is not NULL */
    assert(pcKey != NULL);

    
    while (*pcKey != '\0') {
        hash = (hash << HASH_SHIFT_AMOUNT) + (unsigned int)(*pcKey++);
    }
    return hash % bucketCount;
}

/* Sets up a new, empty symbol table.
   No arguments, just initializes the structure, sets up buckets array,
   and returns a pointer to the table or NULL if there's an allocation issue. */
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

/* Releases all memory used by the symbol table.
   Frees up each key-value node and the main structure itself.
   Arguments -> `oSymTable`: the symbol table to be freed
   Doesn't return anything but needs a valid SymTable pointer. */
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

/*
 * Gives the total number of key-value pairs in the table.
 * Arguments:
 *   - `oSymTable`: the symbol table to check
 * Returns the number of bindings in the table as `size_t`.
 */
size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->nodeQuantity;
}

/*
 * Expands the hash table if it becomes too full (load factor is exceeded).
 * Arguments:
 *   - `oSymTable`: the symbol table to resize
 * Sets up a larger bucket array and redistributes all nodes into new buckets.
 * If memory allocation fails, it leaves the table unchanged.
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

            /* Use the new bucket count for rehashing */
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

/*
 * Adds a new key-value pair to the symbol table if the key doesn’t already exist.
 * Arguments:
 *   - `oSymTable`: the symbol table
 *   - `pcKey`: string key to add
 *   - `pvValue`: the value associated with `pcKey`
 * Checks if the key exists, then allocates a new node and inserts it.
 * Resizes the table if it gets too full. Returns integer, either 1 on success, 0 on failure or if key exists.
 */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    unsigned int index;
    struct SymTableNode *psNewNode, *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    

    /* Check if resizing is needed */
    if ((double)oSymTable->nodeQuantity / oSymTable->bucketCount > LOAD_FACTOR_THRESHOLD) {
        symtablehash_resizeHashTable(oSymTable);
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

/*
 * Replaces the value of an existing key in the table.
 * Arguments:
 *   - `oSymTable`: the symbol table
 *   - `pcKey`: the key whose value we want to update
 *   - `pvValue`: the new value to store
 * Finds the key, updates its value if found, and returns the old value. Returns NULL if the key doesn’t exist.
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

/* 
 * SymTable_contains:
 * Checks if the SymTable contains a specified key.
 * Returns integer, either 1 if the key is found, 0 otherwise.
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

/*
 * Gets the value associated with a specific key.
 * Arguments:
 *   - `oSymTable`: the symbol table
 *   - `pcKey`: the key whose value we want to retrieve
 * Finds and returns the value if `pcKey` exists in the table; returns NULL if the key isn’t found.
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
