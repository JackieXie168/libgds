// ==================================================================
// @(#)hash.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 03/12/2004
// @lastdate 29/09/2006
// ==================================================================

/* This code implements a hash table structure. We can insert same 
 * elements as many as it is needed. It is managed with a reference 
 * counter.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <libgds/array.h>
#include <libgds/memory.h>
#include <libgds/log.h>
#include <libgds/hash.h>

typedef struct {
  FHashEltCompare fEltCompare;
  FHashEltDestroy fEltDestroy;
  FHashCompute fHashCompute;
}SHashFunctions;

struct HashTable {
  uint32_t uHashSize;
  uint32_t uEltCount;
  float fResizeThreshold;
  SHashFunctions * pFunctions;
  SPtrArray ** aHash;
};

typedef struct {
  SHashFunctions * pFunctions;
  void * pElt;
  uint32_t uRef;
}SHashElt;

// ----- _hash_element_compare --------------------------------------
/**
 *
 */
static int _hash_element_compare(void * pHElt1, void * pHElt2,
				 unsigned int uEltSize)
{
  SHashElt * pHashElt1= *((SHashElt **) pHElt1);
  SHashElt * pHashElt2= *((SHashElt **) pHElt2);

  return pHashElt1->pFunctions->fEltCompare(pHashElt1->pElt,
					    pHashElt2->pElt, uEltSize);
}

// ----- _hash_element_remove ---------------------------------------
/**
 *
 */
/*static void _hash_element_remove(void * pHelt)
{
  SHashElt * pHashElt= *((SHashElt **)pHelt);

  if (pHashElt != NULL)
    FREE(pHashElt);
    }*/

// ----- _hash_element_destroy --------------------------------------
/**
 *
 */
static void _hash_element_destroy(void * pHElt)
{
  SHashElt * pHashElt= *((SHashElt **)pHElt);

  if (pHashElt->pFunctions->fEltDestroy != NULL)
    pHashElt->pFunctions->fEltDestroy(pHashElt->pElt);
  FREE(pHashElt);
}

// ----- _hash_element_unref ----------------------------------------
/**
 *
 */
static int _hash_element_unref(void * pHElt)
{
  SHashElt * pHashElt= *((SHashElt **)pHElt);

  pHashElt->uRef--;
  return pHashElt->uRef;
}

// ----- _hash_element_ref ------------------------------------------
/**
 *
 */
static int _hash_element_ref(SHashElt * pHashElt)
{
  pHashElt->uRef++;
  return pHashElt->uRef;
}

// ----- _hash_element_init -----------------------------------------
/**
 *
 */
static SHashElt * _hash_element_init(void * pElt, SHashFunctions * pFunctions)
{
  SHashElt * pHashElt= MALLOC(sizeof(SHashElt));

  pHashElt->pFunctions= pFunctions;
  pHashElt->pElt= pElt;
  pHashElt->uRef= 0;
  return pHashElt;
}

// ----- _hash_element_search ---------------------------------------
/**
 * RETURNS:
 *   -1 if element could not be found.
 *   0 if element was found. In this case, *puIndex will contain the
 *      index of the element.
 */
static int _hash_element_search(SPtrArray * aHashElts, void * pElt, 
				SHashFunctions * pFunctions,
				unsigned int * puIndex)
{
  unsigned int uIndex;
  SHashElt * pHashElt= MALLOC(sizeof(SHashElt));

  pHashElt->pElt= pElt;
  pHashElt->pFunctions= pFunctions;
  if (ptr_array_sorted_find_index(aHashElts, &pHashElt, &uIndex) == -1) {
    FREE(pHashElt);
    return -1;
  }
  
  FREE(pHashElt);

  *puIndex= uIndex;
  return 0;
}

// ----- _hash_element_add ------------------------------------------
/**
 *
 */
static SHashElt * _hash_element_add(SPtrArray * aHashElt, void * pElt, 
				    SHashFunctions * pFunctions)
{
  SHashElt * pHashElt= _hash_element_init(pElt, pFunctions);
  ptr_array_add(aHashElt, &pHashElt);
  return pHashElt;
}

// ----- hash_init ---------------------------------------------------
/**
 *
 */
SHash * hash_init(const uint32_t uHashSize, float fResizeThreshold, 
		  FHashEltCompare fEltCompare, 
		  FHashEltDestroy fEltDestroy, 
		  FHashCompute fHashCompute)
{
  SHash * pHash= MALLOC(sizeof(SHash));

  pHash->aHash= MALLOC(sizeof(void *)*uHashSize);
  memset(pHash->aHash, 0, sizeof(void *)*uHashSize);

  assert(fHashCompute != NULL);

  pHash->pFunctions= MALLOC(sizeof(SHashFunctions));
  pHash->pFunctions->fEltCompare= fEltCompare;
  pHash->pFunctions->fEltDestroy= fEltDestroy;
  pHash->pFunctions->fHashCompute= fHashCompute;

  pHash->uHashSize= uHashSize;
  pHash->fResizeThreshold= fResizeThreshold;
  pHash->uEltCount= 0;

  return pHash;
}


// ----- hash_rehash -------------------------------------------------
/**
 *
 */
/*void hash_rehash(SHash * pHash)
{
  uint32_t uHashKey, uArrayIter, uNewHashSize;
  SPtrArray * aHashElt;
  SHashElt * pHashElt;
  void * pElt;
  
  uNewHashSize = pHash->uHashSize*2;
  pHash->aHash = REALLOC(pHash->aHash, sizeof(uint32_t)*uNewHashSize);
  memset((pHash->aHash)+pHash->uHashSize, 0, sizeof(uint32_t)*pHash->uHashSize);
  pHash->uHashSize = uNewHashSize;

  for (uHashKey = 0; uHashKey < pHash->uHashSize; uHashKey++) {
    if ( (aHashElt = pHash->aHash[uHashKey]) != NULL) {
      for (uArrayIter = 0; uArrayIter < ptr_array_length(aHashElt); uArrayIter++) {
	pHashElt = aHashElt->data[uArrayIter];
	if (pHashElt->uKeyNonModulo > pHash->uHashSize) {
	  pElt = pHashElt->pElt;
	  //Tricky part !
	  //hash_del is not the good method. pElt has to be moved and not unref or deleted.
	  //Thus, since we free the memory of an element when we delete it from the array. We change
	  //temporarily the function to not free the memory of the element in the hash when we move it
	  //but we only free the structure in which it has been inserted.
	  ptr_array_set_fdestroy(aHashElt, hash_element_remove);
	  ptr_array_remove_at(aHashElt, uArrayIter);
	  ptr_array_set_fdestroy(aHashElt, hash_element_destroy);
	  hash_add(pHash, pElt);
	} 
      }
    }
  }

  }*/

// ----- hash_add ----------------------------------------------------
/**
 *
 * returns -1 if error else the key at which the elt has been added
 */
int hash_add(SHash * pHash, void * pElt)
{
  uint32_t uHashKey;
  unsigned int uIndex= 0;
  SHashElt * pHashElt;
  SPtrArray * aHashElt;

  uHashKey= pHash->pFunctions->fHashCompute(pElt, pHash->uHashSize);
  assert(uHashKey < pHash->uHashSize);
  
  if ((aHashElt= pHash->aHash[uHashKey]) == NULL)
    aHashElt= pHash->aHash[uHashKey]=
      ptr_array_create(ARRAY_OPTION_UNIQUE|ARRAY_OPTION_SORTED,
		       _hash_element_compare,
		       _hash_element_destroy);

  if (_hash_element_search(aHashElt, pElt, pHash->pFunctions, &uIndex) == -1) {
  //  if (++pHash->uEltCount >
  //      (uint32_t)((float)pHash->uHashSize*pHash->fResizeThreshold)) {
  //    hash_rehash(pHash);
  //  }
    pHashElt= _hash_element_add(aHashElt, pElt, pHash->pFunctions);
  } else {
    pHashElt= aHashElt->data[uIndex];
  }

  _hash_element_ref(pHashElt);
  return uHashKey;
}

// ----- hash_search -------------------------------------------------
/**
 * returns NULL if no elt found else the pointer to this elt
 */
void * hash_search(SHash * pHash, void * pElt)
{
  unsigned int uIndex;
  SPtrArray * aHashElts;
  SHashElt * pHashEltSearched = NULL;
  uint32_t uHashKey= pHash->pFunctions->fHashCompute(pElt, pHash->uHashSize);

  if ((aHashElts = pHash->aHash[uHashKey]) != NULL) {
    if (_hash_element_search(aHashElts, pElt, pHash->pFunctions,
			    &uIndex) != -1) 
      pHashEltSearched= aHashElts->data[uIndex];
  }

  return (pHashEltSearched == NULL) ? NULL : pHashEltSearched->pElt;
}

// ----- hash_del ----------------------------------------------------
/**
 *  returns 0 if no elt deleted (unreferenced) else 1 if unreferenced 
 *  otherwise 2 if deleted
 */
int hash_del(SHash * pHash, void * pElt)
{
  uint32_t uHashKey= pHash->pFunctions->fHashCompute(pElt, pHash->uHashSize);
  SPtrArray * aHashElt;
  int iRet = 0;
  unsigned int uIndex;
  
  if ( (aHashElt = pHash->aHash[uHashKey]) != NULL) {
    if (_hash_element_search(aHashElt, pElt, pHash->pFunctions,
			    &uIndex) != -1) {
      iRet= 1;
      if (_hash_element_unref(&aHashElt->data[uIndex]) <= 0){
	iRet= 2;
	pHash->uEltCount--;
	ptr_array_remove_at(aHashElt, uIndex);
      }
    }
  }
  if (iRet == 0)
    fprintf(stderr, "hash_del> No elt unreferenced.\n");

  return iRet;
}

// -----[ hash_info ]------------------------------------------------
/**
 * Return number of references on the given item.
 */
uint32_t hash_info(SHash * pHash, void * pItem)
{
  unsigned int uIndex;
  SPtrArray * pHashItems;
  SHashElt * pSearched= NULL;
  uint32_t uHashKey;

  uHashKey= pHash->pFunctions->fHashCompute(pItem, pHash->uHashSize);
  pHashItems= pHash->aHash[uHashKey];
  if (pHashItems == NULL)
    return 0;

  if (_hash_element_search(pHashItems, pItem, pHash->pFunctions,
			   &uIndex) == -1)
    return 0;

  pSearched= pHashItems->data[uIndex];
  return pSearched->uRef;
}

// -----[ hash_for_each_key ]----------------------------------------
/**
 * Call the given callback function foreach key in the hash table. The
 * item which is passed to the callback function is the array at the
 * given key. This makes possible to evaluate the hash function. If
 * the distribution is quite uniform, the hash function is
 * good. Otherwise, it is time to look for another one.
 *
 * Note: the callback can be called with an item which is NULL.
 */
int hash_for_each_key(SHash * pHash, FHashForEach fHashForEach, 
		      void * pContext)
{
  int iResult;
  uint32_t uHashKey;
  SPtrArray * pHashItems;

  for (uHashKey= 0; uHashKey < pHash->uHashSize; uHashKey++) {
    pHashItems= pHash->aHash[uHashKey];
    iResult= fHashForEach(pHashItems, pContext);
    if (iResult < 0)
      return iResult;
  }
  return 0;
}

// -----[ hash_for_each ]--------------------------------------------
/**
 * Call the given callback function foreach item in the hash table.
 */
int hash_for_each(SHash * pHash, FHashForEach fHashForEach, 
		  void * pContext)
{
  int iResult;
  uint32_t uHashKey, uIndex;
  SPtrArray * pHashItems;
  void * pItem;

  for (uHashKey= 0; uHashKey < pHash->uHashSize; uHashKey++) {
    pHashItems= pHash->aHash[uHashKey];
    if (pHashItems != NULL) {
      for (uIndex= 0; uIndex < ptr_array_length(pHashItems); uIndex++) {
	pItem= ((SHashElt *) pHashItems->data[uIndex])->pElt;
	iResult= fHashForEach(pItem, pContext);
	if (iResult < 0)
	  return iResult;
      }
    }
  }
  return 0;
}

// ----- hash_destroy ------------------------------------------------
/**
 *
 *
 */
void hash_destroy(SHash ** pHash)
{
  uint32_t uCpt;
  if (*pHash != NULL) {
    for (uCpt = 0; uCpt < (*pHash)->uHashSize; uCpt++) {
      if ( (*pHash)->aHash[uCpt] != NULL)
	ptr_array_destroy(&((*pHash)->aHash[uCpt]));
    }
    
    if ( (*pHash)->pFunctions != NULL)
      FREE( (*pHash)->pFunctions);

    FREE( (*pHash)->aHash );
    FREE( (*pHash) );
    (*pHash) = NULL;
  } 
}

/////////////////////////////////////////////////////////////////////
//
// ENUMERATION
//
/////////////////////////////////////////////////////////////////////

// -----[ SHashEnumContext ]-----------------------------------------
typedef struct {
  unsigned int uIndex1;
  unsigned int uIndex2;
  SHash * pHash;
} SHashEnumContext;

// -----[ _hash_get_enum_has_next ]----------------------------------
int _hash_get_enum_has_next(void * pContext)
{
  SHashEnumContext * pHashContext= (SHashEnumContext *) pContext;
  SPtrArray * pHashItems;
  if (pHashContext->uIndex1 >= pHashContext->pHash->uHashSize )
    return 0;
  pHashItems= pHashContext->pHash->aHash[pHashContext->uIndex1];
  if (pHashItems == NULL)
    return 0;
  return (pHashContext->uIndex2 < ptr_array_length(pHashItems));
}

// -----[ _hash_get_enum_get_next ]----------------------------------
void * _hash_get_enum_get_next(void * pContext)
{
  SHashEnumContext * pHashContext= (SHashEnumContext *) pContext;
  SPtrArray * pHashItems= pHashContext->pHash->aHash[pHashContext->uIndex1];
  SHashElt * pItem= (SHashElt *) pHashItems->data[pHashContext->uIndex2];
  if (pHashContext->uIndex2+1 < ptr_array_length(pHashItems)) {
    pHashContext->uIndex2++;
  } else {
    pHashContext->uIndex1++;
    pHashContext->uIndex2= 0;
  }
  return pItem->pElt;
}

// -----[ _hash_get_enum_destroy ]-----------------------------------
void _hash_get_enum_destroy(void * pContext)
{
  SHashEnumContext * pHashContext= (SHashEnumContext *) pContext;
  FREE(pHashContext);
}

// -----[ hash_get_enum ]--------------------------------------------
SEnumerator * hash_get_enum(SHash * pHash)
{
  SHashEnumContext * pContext=
    (SHashEnumContext *) MALLOC(sizeof(SHashEnumContext));
  pContext->uIndex1= 0;
  pContext->uIndex2= 0;
  pContext->pHash= pHash;
  return enum_create(pContext,
		     _hash_get_enum_has_next,
		     _hash_get_enum_get_next,
		     _hash_get_enum_destroy);
}
