// ==================================================================
// @(#)assoc_array.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 05/01/2007
// @lastdate 17/03/2007
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <libgds/assoc_array.h>
#include <libgds/memory.h>
#include <libgds/str_util.h>

typedef struct {
  char * pcKey;
  void * pValue;
} SAssocItem;

// -----[ _assoc_item_create ]---------------------------------------
static inline SAssocItem * _assoc_item_create(const char * pcKey,
					      void * pValue)
{
  SAssocItem * pItem= (SAssocItem *) MALLOC(sizeof(SAssocItem));
  pItem->pcKey= str_create(pcKey);
  pItem->pValue= pValue;
  return pItem;
}

// -----[ _assoc_item_destroy ]--------------------------------------
static inline void _assoc_item_destroy(SAssocItem ** ppItem)
{
  if (*ppItem != NULL) {
    str_destroy(&(*ppItem)->pcKey);
    // SHALL WE DESTROY ASSOCIATED ITEM ? LATER...
    // WILL CAUSE TEMPORARY MEMORY LEAKS...
    FREE(*ppItem);
    *ppItem= NULL;
  }
}

// -----[ _assoc_array_item_compare ]--------------------------------
/**
 * Private helper function used to compare 2 keys.
 */
static int _assoc_array_item_compare(void * pItem1, void * pItem2,
				     unsigned int uEltSize)
{
  SAssocItem * pAssocItem1= *((SAssocItem **) pItem1);
  SAssocItem * pAssocItem2= *((SAssocItem **) pItem2);

  return strcmp(pAssocItem1->pcKey, pAssocItem2->pcKey);
}

// -----[ _assoc_array_item_destroy ]--------------------------------
/**
 * Private helper function used to destroy each item.
 */
static void _assoc_array_item_destroy(void * pItem)
{
  _assoc_item_destroy((SAssocItem **) pItem);
}

// -----[ assoc_array_create ]---------------------------------------
SAssocArray * assoc_array_create()
{
  return (SAssocArray *) ptr_array_create(ARRAY_OPTION_SORTED,
					  _assoc_array_item_compare,
					  _assoc_array_item_destroy);
}

// -----[ assoc_array_destroy ]--------------------------------------
void assoc_array_destroy(SAssocArray ** ppArray)
{
  ptr_array_destroy(ppArray);
}

// -----[ assoc_array_length ]-------------------------------------
unsigned int assoc_array_length(SAssocArray * pArray)
{
  return ptr_array_length(pArray);
}

// -----[ assoc_array_exists ]---------------------------------------
int assoc_array_exists(SAssocArray * pArray, const char * pcKey)
{
  SAssocItem sTmp;
  SAssocItem * pTmp= &sTmp;
  unsigned int uIndex;

  sTmp.pcKey= (char *) pcKey;
  if (!ptr_array_sorted_find_index(pArray, &pTmp, &uIndex))
    return 1;
  return 0;
}

// -----[ assoc_array_get ]------------------------------------------
void * assoc_array_get(SAssocArray * pArray, const char * pcKey)
{
  SAssocItem sTmp;
  SAssocItem * pTmp= &sTmp;
  unsigned int uIndex;

  sTmp.pcKey= (char *) pcKey;
  if (!ptr_array_sorted_find_index(pArray, &pTmp, &uIndex))
    return ((SAssocItem *) pArray->data[uIndex])->pValue;
  return NULL;
}

// -----[ assoc_array_set ]------------------------------------------
/**
 * RETURNS:
 *    0 in case of success
 *   -1 in case of failure
 */
int assoc_array_set(SAssocArray * pArray, const char * pcKey,
		     void * pValue)
{
  SAssocItem * pItem= _assoc_item_create(pcKey, pValue);
  if (ptr_array_add(pArray, &pItem) < 0) {
    return -1;
  }
  return 0;
}

// -----[ assoc_array_for_each ]-------------------------------------
int assoc_array_for_each(SAssocArray * pArray, FAssocArrayForEach fForEach,
			 void * pContext)
{
  unsigned int uIndex;
  SAssocItem * pItem;
  int iResult;

  for (uIndex= 0; uIndex < ptr_array_length(pArray); uIndex++) {
    pItem= (SAssocItem *) pArray->data[uIndex];
    iResult= fForEach(pItem->pcKey, pItem->pValue, pContext);
    if (iResult)
      return iResult;
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////
//
// ENUMERATOR
//
/////////////////////////////////////////////////////////////////////

// -----[ SEnumContext ]---------------------------------------------
typedef struct {
  unsigned int uIndex;
  SAssocArray * pArray;
} SEnumContext;

// -----[ _enum_has_next ]-------------------------------------------
static int _enum_has_next(void * pContext)
{
  SEnumContext * pEnumContext= (SEnumContext *) pContext;
  return (pEnumContext->uIndex
	  < _array_length((SArray *) pEnumContext->pArray));
}

// -----[ _enum_get_next ]-------------------------------------------
static void * _enum_get_next(void * pContext)
{
  SEnumContext * pEnumContext;
  SAssocItem * pItem;
  
  pEnumContext= (SEnumContext *) pContext;
  _array_get_at((SArray *) pEnumContext->pArray,
		pEnumContext->uIndex, &pItem);
  return pItem->pcKey;
}

// -----[ _enum_destroy ]--------------------------------------------
static void _enum_destroy(void * pContext)
{
  SEnumContext * pEnumContext= (SEnumContext *) pContext;
  FREE(pEnumContext);
}

// -----[ assoc_array_get_enum ]-------------------------------------
SEnumerator * assoc_array_get_enum(SAssocArray * pArray)
{
  SEnumContext * pContext=
    (SEnumContext *) MALLOC(sizeof(SEnumContext));
  pContext->pArray= pArray;
  pContext->uIndex= 0;
  return enum_create(pContext,
		     _enum_has_next,
		     _enum_get_next,
		     _enum_destroy);
}
