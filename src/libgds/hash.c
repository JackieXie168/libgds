// ==================================================================
// @(#)hash.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 03/12/2004
// @lastdate 17/01/2007
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
  uint32_t uCurrentKey;
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
static void _hash_element_remove(void * pHelt)
{
  SHashElt * pHashElt= *((SHashElt **)pHelt);

  if (pHashElt != NULL)
    FREE(pHashElt);
}

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
static int _hash_element_search(const SPtrArray * aHashElts, void * pElt, 
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

static void _hash_element_set_current_key(SHashElt * pHashElt, const uint32_t uHashKey)
{
  pHashElt->uCurrentKey = uHashKey;
}

static uint32_t _hash_element_get_current_key(const SHashElt * pHashElt)
{
  return pHashElt->uCurrentKey;
}

// ----- _hash_element_add ------------------------------------------
/**
 *
 */
static SHashElt * _hash_element_add(const SPtrArray * aHashElt, void * pElt, 
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
SHash * hash_init(const uint32_t uHashSize, const float fResizeThreshold, 
		  FHashEltCompare fEltCompare, 
		  FHashEltDestroy fEltDestroy, 
		  FHashCompute fHashCompute)
{
  SHash * pHash= MALLOC(sizeof(SHash));

  pHash->aHash= MALLOC(sizeof(void *)*uHashSize);
  memset(pHash->aHash, 0, sizeof(void *)*uHashSize);

  assert(fHashCompute != NULL);
  assert(fResizeThreshold >= 0.0);
  assert(fResizeThreshold < 1.0);

  pHash->pFunctions= MALLOC(sizeof(SHashFunctions));
  pHash->pFunctions->fEltCompare= fEltCompare;
  pHash->pFunctions->fEltDestroy= fEltDestroy;
  pHash->pFunctions->fHashCompute= fHashCompute;

  pHash->uHashSize= uHashSize;
  pHash->fResizeThreshold= fResizeThreshold;
  pHash->uEltCount= 0;

  return pHash;
}


static SPtrArray * _hash_get_hash_array(SHash * pHash, uint32_t uHashKey)
{
  assert(uHashKey < pHash->uHashSize);

  if (pHash->aHash[uHashKey] == NULL) {
    pHash->aHash[uHashKey]= ptr_array_create(
	ARRAY_OPTION_UNIQUE|ARRAY_OPTION_SORTED,
		       _hash_element_compare,
		       _hash_element_destroy);
  }
  return pHash->aHash[uHashKey];
}

static uint32_t _hash_compute_key(const SHash * pHash, const void * pElt)
{
  return pHash->pFunctions->fHashCompute(pElt, pHash->uHashSize);
}

// ----- hash_rehash -------------------------------------------------
/**
 * 
 */
static void _hash_rehash(SHash * pHash)
{
  uint32_t uHashKey;
  uint32_t uIndex;
  uint32_t uArrayIter;
  uint32_t uNewHashSize;
  uint32_t uOldHashSize;
  SPtrArray * aHashElt;
  SPtrArray * aHashEltNew;
  SHashElt * pHashElt;
  void * pElt= NULL;
  
  uNewHashSize = pHash->uHashSize*2;
  pHash->aHash = REALLOC(pHash->aHash, sizeof(uint32_t)*uNewHashSize);
  uOldHashSize = pHash->uHashSize;
  memset((pHash->aHash)+uOldHashSize, 0, sizeof(uint32_t)*pHash->uHashSize);
  pHash->uHashSize = uNewHashSize;

  for (uIndex = 0; uIndex < uOldHashSize; uIndex++) {
    if ( (aHashElt = pHash->aHash[uIndex]) != NULL) {
      for (uArrayIter = 0; uArrayIter < ptr_array_length(aHashElt); uArrayIter++) {
	// Extract the Elt
	pHashElt = aHashElt->data[uArrayIter];
	uHashKey = _hash_compute_key(pHash, pElt);
	/* If the old key is the same as the new. Don't move the pHashElt as it
	 * stays in the same array of the hash table */
	if (uHashKey != _hash_element_get_current_key(pHashElt)) {
	  pElt = pHashElt->pElt;
	  // Remove the element
	  /* change the destroy function in the array structure as we don't
	   * want the pElt to be destroyed */
	  ptr_array_set_fdestroy(aHashElt, _hash_element_remove);
	  ptr_array_remove_at(aHashElt, uArrayIter);
	  // Get his new aHashElt array in which setting pElt
	  aHashEltNew = _hash_get_hash_array(pHash, uHashKey);
	  pHashElt = _hash_element_add(aHashEltNew, pElt, pHash->pFunctions);
	  /* Change the current key of the Elt */
	  _hash_element_set_current_key(pHashElt, uHashKey);
	  /* restore the destroy function */
	  ptr_array_set_fdestroy(aHashEltNew, _hash_element_destroy);
	} 
      } 
    }
  }
}


// ----- hash_add ----------------------------------------------------
/**
 *
 * returns -1 if error else the key at which the elt has been added
 *
 */
int hash_add(SHash * pHash, void * pElt)
{
  unsigned int uIndex= 0;
  SHashElt * pHashElt;
  SPtrArray * aHashElt;
  uint32_t uHashKey;

  uHashKey = _hash_compute_key(pHash, pElt);
  aHashElt = _hash_get_hash_array(pHash, uHashKey);

  if (_hash_element_search(aHashElt, pElt, pHash->pFunctions, &uIndex) == -1) {
    if (pHash->fResizeThreshold != 0.0) {
      if (++pHash->uEltCount >
	  (uint32_t)((float)pHash->uHashSize*pHash->fResizeThreshold)) {
	_hash_rehash(pHash);
	/* Get the new array of the hash table to add the elt as the hash table
	 * size has changed */
	uHashKey = _hash_compute_key(pHash, pElt);
	aHashElt = _hash_get_hash_array(pHash, uHashKey);
      }
    }
    pHashElt= _hash_element_add(aHashElt, pElt, pHash->pFunctions);
    _hash_element_set_current_key(pHashElt, uHashKey);
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
void * hash_search(const SHash * pHash, void * pElt)
{
  unsigned int uIndex;
  SPtrArray * aHashElts;
  SHashElt * pHashEltSearched = NULL;
  uint32_t uHashKey= pHash->pFunctions->fHashCompute(pElt, pHash->uHashSize);

  if ((aHashElts = pHash->aHash[uHashKey]) != NULL) {
    if (_hash_element_search(aHashElts, pElt, pHash->pFunctions,
			    &uIndex) != -1) 
      _array_get_at((SArray*)aHashElts, uIndex, &pHashEltSearched);
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
/*  if (iRet == 0)
    fprintf(stderr, "hash_del> No elt unreferenced.\n");*/

  return iRet;
}

// -----[ hash_info ]------------------------------------------------
/**
 * Return number of references on the given item.
 */
uint32_t hash_info(const SHash * pHash, void * pItem)
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
int hash_for_each_key(const SHash * pHash, FHashForEach fHashForEach, 
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
int hash_for_each(const SHash * pHash, FHashForEach fHashForEach, 
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
  uint32_t uIndex1;


  uIndex1 = pHashContext->uIndex1;
  /* First, have a look at the index we are in the hash table */
  /* If it is not satisfying we *have* to get to the place of the potential
   * next item which can "far away" from the last */

  /* Have we already get all the items? */
  if (uIndex1 >= pHashContext->pHash->uHashSize )
    return 0;

  /* is the anything more in the array we are going through? */
  pHashItems = pHashContext->pHash->aHash[pHashContext->uIndex1];
  if (pHashItems != NULL && pHashContext->uIndex2 < ptr_array_length(pHashItems))
    return 1;

  uIndex1++;
  /* We have to find in the following arrays for the next item */
  for (; uIndex1 < pHashContext->pHash->uHashSize; uIndex1++) {
    pHashItems = pHashContext->pHash->aHash[uIndex1];
    if (pHashItems != NULL && ptr_array_length(pHashItems) > 0) 
      return 1;
  }
  return 0;
}

// -----[ _hash_get_enum_get_next ]----------------------------------
void * _hash_get_enum_get_next(void * pContext)
{
  SHashEnumContext * pHashContext= (SHashEnumContext *) pContext;
  SPtrArray * pHashItems= NULL;
  void * pItem= NULL; 

  /* Have we already get all the items? */
  if (pHashContext->uIndex1 >= pHashContext->pHash->uHashSize)
    return NULL;

  /* is the anything more in the array we are going through? */
  pHashItems = pHashContext->pHash->aHash[pHashContext->uIndex1];
  if (pHashItems != NULL && pHashContext->uIndex2 < ptr_array_length(pHashItems)) {
    pItem = ((SHashElt *)pHashItems->data[pHashContext->uIndex2])->pElt;
    pHashContext->uIndex2++;
    return pItem;
  }

  /* We have to return an item from another array of the hash table */
  pHashContext->uIndex1++;
  /* We may initialize it to 1 as we've got to return the first element anyway */
  pHashContext->uIndex2= 1;
  for (; pHashContext->uIndex1 < pHashContext->pHash->uHashSize; pHashContext->uIndex1++) {
    pHashItems = pHashContext->pHash->aHash[pHashContext->uIndex1];
    if (pHashItems != NULL && ptr_array_length(pHashItems) > 0) {
      return ((SHashElt *)pHashItems->data[0])->pElt;
    }
  }
  return NULL;
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
