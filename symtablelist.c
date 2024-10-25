/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: Ephraim Meles                                              */
/*--------------------------------------------------------------------*/


#include <assert.h>
#include <stdlib.h>
#include "symtable.h"
#include <string.h>

/* SymTableNodes consist of keys, values, and the address of the next
node. SymTableNodes form a singly linked list.
form a list.  */

struct SymTableNode
{
   /* The key */
   char *pcKey;

   /* The value. */
   const void *pvValue;

   /* The address of the next SymTableNode. */
   struct SymTableNode *psNextNode;
};

/* A SymTable is a "dummy" node that points to the first SymTableNode. 
This is the managing structure - only needs two fields */

struct SymTable
{
   /* The address of the first SymTableNode. */
   struct SymTableNode *psFirstNode;

   /* Number of nodes in linked list */
   size_t nodeQuantity;
};

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

/* Takes in oSymTable, returns its number of bindings. */

size_t SymTable_getLength(SymTable_T oSymTable)
{
   
   assert(oSymTable != NULL);

return oSymTable-> nodeQuantity;
}

/*--------------------------------------------------------------------*/

/* Takes in oSymTable and frees all the memory that it occupies. */

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

/*--------------------------------------------------------------------*/

/* Returns 1 (TRUE) if oSymTable does not contain a binding with
key pcKey. Returns 0 (FALSE) if either there is insufficient memory
or oSymTable contains any binding with key pcKey. */

int SymTable_put(SymTable_T oSymTable, 
const char *pcKey, const void *pvValue)
{
   struct SymTableNode *psNewNode;
   /* Travelling, placeholder node */
   struct SymTableNode *psCurrentNode;
   assert(pvValue != NULL);
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

/*--------------------------------------------------------------------*/

/* In the bindings of oSymTable with a key equal to pcKey, replace 
the binding's value and RETURN the previous value. Otherwise,
return NULL, table remains unchanged. */

void *SymTable_replace(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue)
{
   struct SymTableNode* psCurrentNode;
   void *oldValue;
   
   assert(pvValue != NULL);
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

/*--------------------------------------------------------------------*/

/* Returns 1(TRUE) if oSymTable contains a binding with a key
equal to pcKey. Otherwise return 0(FALSE). */

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

/*--------------------------------------------------------------------*/

/* Returns the value of the binding within oSymTable with a key equal
to pcKey. Otherwise return NULL. */

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

/*--------------------------------------------------------------------*/

/* Removes bindings in oSymTable with key == pcKey and RETURNS
their value. Otherwise, return NULL and leave oSymTable 
untouched. */

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

/*--------------------------------------------------------------------*/

/* Applys function *pfApply to each binding in SymTable, passing
pvExtra and calling (*pfApply)(pcKey, pvValue, pvExtra) for 
each pcKey/pvValue binding in oSymTable. */

 void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra)
{
   struct SymTableNode *psCurrentNode;
   assert(pvExtra != NULL); 
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
