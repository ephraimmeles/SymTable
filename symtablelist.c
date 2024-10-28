/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: Ephraim Meles                                              */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include "symtable.h"
#include <string.h>
/*
 * SymTableNode: Represents a single entry in the symbol table.
 * Each node has a key-value pair and a pointer to the next node.
 * These nodes are linked together to form a singly linked list.
 */
struct SymTableNode
{
   /* The key */
   char *pcKey;

   /* The value. */
   const void *pvValue;

   /* The address of the next SymTableNode. */
   struct SymTableNode *psNextNode;
};

/*
 * SymTable: Main structure for managing the symbol table.
 * Contains the head pointer to the list of SymTableNodes and a count of nodes.
 * Acts as the entry point to access the linked list of key-value pairs.
 */
struct SymTable
{
   /* The address of the first SymTableNode. */
   struct SymTableNode *psFirstNode;

   /* Number of nodes in linked list */
   size_t nodeQuantity;
};

/*
 * SymTable_new:
 * Creates an empty symbol table.
 * Arguments: None
 * Behavior: Allocates memory for a new `SymTable` structure, initializes
 *           `psFirstNode` to NULL, and sets `nodeQuantity` to 0.
 * Returns: A pointer to the new SymTable, or NULL if memory allocation fails.
 */
SymTable_T SymTable_new(void)
{
   SymTable_T oSymTable;

   oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
   if (oSymTable == NULL)
      return NULL;

   oSymTable->psFirstNode = NULL;
   oSymTable-> nodeQuantity = 0;
   return oSymTable;
}

/*
 * SymTable_getLength:
 * Returns the number of key-value pairs in the symbol table.
 * Arguments:
 *   - `oSymTable`: pointer to the symbol table
 * Behavior: Checks the `nodeQuantity` field of the table to get the count.
 * Returns: The total number of bindings in the table as a `size_t`.
 */
size_t SymTable_getLength(SymTable_T oSymTable)
{
   
   assert(oSymTable != NULL);

return oSymTable-> nodeQuantity;
}

/*
 * SymTable_free:
 * Frees all memory associated with the symbol table.
 * Arguments:
 *   - `oSymTable`: pointer to the symbol table to free
 * Behavior: Iterates through each node in the linked list, frees each key 
 *           and node, and finally frees the table itself.
 * Returns: Nothing (void).
 */
void SymTable_free(SymTable_T oSymTable)
{
   struct SymTableNode *psCurrentNode;
   struct SymTableNode *psNextNode;

   assert(oSymTable != NULL);

   for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psNextNode)
   {
      psNextNode = psCurrentNode->psNextNode;
      
   /* free the current node's key, const char stars are protected,
   must cast as plain char star to free. */
      free((char*)psCurrentNode->pcKey);
      free(psCurrentNode);
   }
   free(oSymTable);
}

/*
 * SymTable_put:
 * Adds a new key-value pair to the symbol table if the key is unique.
 * Arguments:
 *   - `oSymTable`: pointer to the symbol table
 *   - `pcKey`: string key for the new binding
 *   - `pvValue`: pointer to the value associated with the key
 * Behavior: Checks if `pcKey` already exists. If not, allocates a new node
 *           with `pcKey` and `pvValue` and adds it to the front of the list.
 * Returns: 1 if added successfully, 0 if the key already exists or if memory fails.
 */
int SymTable_put(SymTable_T oSymTable, 
const char *pcKey, const void *pvValue)
{
   struct SymTableNode *psNewNode;
   /* Travelling, placeholder node */
   struct SymTableNode *psCurrentNode;
   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

/* WALK for loop */
   for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
      /* pcCurrentNode walks to it's OWN next node */
        psCurrentNode = psCurrentNode->psNextNode)
   {
      if (strcmp(pcKey, psCurrentNode->pcKey) == 0)
      {
         return 0;
      }
   }
   
   /* in the case that there are no duplicates, make space 
   and put in a new node */
   
   psNewNode = (struct SymTableNode*)
      malloc(sizeof(struct SymTableNode));
   if (psNewNode == NULL)
      return 0;
   
   /* need to make space for defensive copy of the key */
   psNewNode->pcKey = malloc(strlen(pcKey) + 1);

   /* check for sufficient memory */
   if (psNewNode->pcKey == NULL)
   {
      free(psNewNode);
      return 0;
   }
   strcpy(psNewNode->pcKey, pcKey);
   oSymTable->nodeQuantity++;
   psNewNode->pvValue = pvValue;
   psNewNode->psNextNode = oSymTable->psFirstNode;
   oSymTable->psFirstNode = psNewNode;
   return 1;
}

/*
 * SymTable_replace:
 * Replaces the value associated with an existing key in the symbol table.
 * Arguments:
 *   - `oSymTable`: pointer to the symbol table
 *   - `pcKey`: string key for the entry to update
 *   - `pvValue`: new value to associate with `pcKey`
 * Behavior: Searches the list for `pcKey`. If found, replaces its value
 *           with `pvValue` and returns the previous value.
 * Returns: The old value if replaced, or NULL if `pcKey` is not found.
 */
void *SymTable_replace(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue)
{
   struct SymTableNode* psCurrentNode;
   void *oldValue;

   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

/* WALK for loop */
   for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
      /* pcCurrentNode walks to it's OWN next node */
        psCurrentNode = psCurrentNode->psNextNode)
   {
      if (strcmp(pcKey, psCurrentNode->pcKey) == 0)
      {
         oldValue = (void*)psCurrentNode->pvValue;
         psCurrentNode->pvValue = pvValue;
         return oldValue;
      }
   }
return NULL;
}

/*
 * SymTable_contains:
 * Checks if the symbol table has an entry with the given key.
 * Arguments:
 *   - `oSymTable`: pointer to the symbol table
 *   - `pcKey`: string key to look for
 * Behavior: Searches the list for `pcKey`.
 * Returns: 1 if `pcKey` exists in the table, 0 otherwise.
 */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{
   struct SymTableNode* psCurrentNode;
   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

/* WALK for loop */
   for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
      /* pcCurrentNode walks to it's OWN next node */
        psCurrentNode = psCurrentNode->psNextNode)
   {
      if (strcmp(pcKey, psCurrentNode->pcKey) == 0)
      {
         return 1;
      }
   }
return 0;
}

/*
 * SymTable_get:
 * Retrieves the value associated with a given key.
 * Arguments:
 *   - `oSymTable`: pointer to the symbol table
 *   - `pcKey`: string key to retrieve
 * Behavior: Searches the list for `pcKey`. If found, returns the value associated with it.
 * Returns: The value if `pcKey` is found, or NULL if `pcKey` does not exist.
 */
  void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{
   struct SymTableNode* psCurrentNode;
   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

/* WALK for loop */
   for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
      /* pcCurrentNode walks to it's OWN next node */
        psCurrentNode = psCurrentNode->psNextNode)
   {
      if (strcmp(pcKey, psCurrentNode->pcKey) == 0)
      {
        return (void*)psCurrentNode->pvValue;
      }
   }
return NULL;
}

/*
 * SymTable_remove:
 * Removes the key-value pair associated with the given key.
 * Arguments:
 *   - `oSymTable`: pointer to the symbol table
 *   - `pcKey`: string key to remove
 * Behavior: Searches for `pcKey`, removes the node if found, adjusts the links,
 *           and decrements `nodeQuantity`.
 * Returns: The removed value if `pcKey` is found, or NULL if `pcKey` does not exist.
 */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{
   struct SymTableNode* psCurrentNode;
   void *oldValue;
   struct SymTableNode* psPrevNode = NULL;
   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

/* WALK for loop */
   for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
      /* pcCurrentNode walks to it's OWN next node */
        psCurrentNode = psCurrentNode->psNextNode)
   {
      if (strcmp(pcKey, psCurrentNode->pcKey) == 0)
      {
      /* save the value */
         oldValue = (void*)psCurrentNode->pvValue;
      /* if else statement to change the links to remove the node */
         if (psPrevNode == NULL)
         {
      /* sym tables first node skip current and go to current's next */
            oSymTable->psFirstNode = psCurrentNode->psNextNode;
         }
         else 
         {
      /* previous node's next has to point to current node's next */
            psPrevNode->psNextNode = psCurrentNode->psNextNode;
         }
         /* free the key and free the node */
         free ((void*)psCurrentNode->pcKey);
         free (psCurrentNode);
         /* decrement count */
         oSymTable->nodeQuantity--;
         /* return value */
         return oldValue;
      }
     /* move previous to become the current one */
      psPrevNode = psCurrentNode;
   }
return NULL;
}

/*
 * SymTable_map:
 * Applies a given function to each key-value pair in the symbol table.
 * Arguments:
 *   - `oSymTable`: pointer to the symbol table
 *   - `pfApply`: function to call on each key-value pair
 *   - `pvExtra`: additional data to pass to `pfApply`
 * Behavior: Iterates through each node in the table and calls `pfApply` 
 *           with the key, value, and `pvExtra`.
 * Returns: Nothing (void).
 */
 void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra)
{
   struct SymTableNode *psCurrentNode;
   
   assert(oSymTable != NULL);
   assert(pfApply != NULL);

   for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psCurrentNode->psNextNode)
      /* pfApply has three arguments, key, value, extra */ 
      (*pfApply)(psCurrentNode->pcKey, 
         (void*)psCurrentNode->pvValue, 
         (void*)pvExtra);
}
