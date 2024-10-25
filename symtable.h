/*--------------------------------------------------------------------*/
/* SymTable.h (Version 6)                                             */
/* Author: Anh Dao                                                    */
/*--------------------------------------------------------------------*/

#ifndef SymTable_INCLUDED
#define SymTable_INCLUDED
#include <stddef.h>

/* Declare ADT SymTable */

typedef struct SymTable *SymTable_T;

/* Returns a SymTable containing no bindings. Returns NULL if
memory is insufficient. */

SymTable_T SymTable_new(void);

/* Takes in oSymTable, returns its number of bindings. */

size_t SymTable_getLength(SymTable_T oSymTable);

/* Takes in oSymTable and frees all the memory that it occupies. */

void SymTable_free(SymTable_T oSymTable);

/* Returns 1 (TRUE) if oSymTable does not contain a binding with
key pcKey. Returns 0 (FALSE) if either there is insufficient 
memoryor oSymTable contains any binding with key pcKey. pvValue
stores value of previous node. */

int SymTable_put(SymTable_T oSymTable,
   const char *pcKey, const void *pvValue);

/* pvValue stores value of previous node.
In the bindings of oSymTable with a key equal to pcKey, replace 
the binding's value and RETURN the previous value. Otherwise,
return NULL, table remains unchanged. */

void *SymTable_replace(SymTable_T oSymTable,
   const char *pcKey, const void *pvValue);

/* Returns 1(TRUE) if oSymTable contains a binding with a key
equal to pcKey. Otherwise return 0(FALSE). */

int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* Returns the value of the binding within oSymTable with a key equal
to pcKey. Otherwise return NULL. */

void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* Removes bindings in oSymTable with key == pcKey and RETURNS
their value. Otherwise, return NULL and leave oSymTable 
untouched. */

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* Applies function *pfApply to each binding in SymTable, passing
pvExtra and calling (*pfApply)(pcKey, pvValue, pvExtra) for 
each pcKey/pvValue binding in oSymTable. */

void SymTable_map(SymTable_T oSymTable,
   void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
   const void *pvExtra);

#endif