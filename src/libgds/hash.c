// ==================================================================
// @(#)hash.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 03/12/2004
// @lastdate 03/12/2004
// ==================================================================

/* This code implements a hash table structure. We can insert same 
 * elements as many as it is needed. It is managed with a reference 
 * counter.
 */

#include <string.h>

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
  SHashFunctions * pFunctions;
  SPtrArray ** aHash;
};

typedef struct {
  SHashFunctions * pFunctions;
  void * pElt;
  uint32_t uRef;
}SHashElt;

// ----- hash_element_compare ----------------------------------------
/**
 *
 *
 */
int hash_element_compare(void * pHElt1, void * pHElt2, uint32_t uEltSize)
{
  SHashElt * pHashElt1 = *((SHashElt **)pHElt1);
  SHashElt * pHashElt2 = *((SHashElt **)pHElt2);

  return pHashElt1->pFunctions->fEltCompare(pHashElt1->pElt, pHashElt2->pElt, uEltSize);
}

// ----- hash_element_destroy ----------------------------------------
/**
 *
 *
 */
void hash_element_destroy(void * pHElt)
{
  SHashElt * pHashElt = *((SHashElt **)pHElt);

  if (pHashElt->pFunctions->fEltDestroy != NULL)
    pHashElt->pFunctions->fEltDestroy(pHashElt->pElt);
}

// ----- hash_element_destroy ----------------------------------------
/**
 *
 *
 */
int hash_element_unref(void * pHElt)
{
  SHashElt * pHashElt = *((SHashElt **)pHElt);

  pHashElt->uRef--;
  return pHashElt->uRef;
}

// ----- hash_element_ref --------------------------------------------
/**
 *
 *
 */
int hash_element_ref(SHashElt * pHashElt)
{
  pHashElt->uRef++;
  return pHashElt->uRef;
}

// ----- hash_element_init -------------------------------------------
/**
 *
 *
 */
SHashElt * hash_element_init(void * pElt, SHashFunctions * pFunctions)
{
  SHashElt * pHashElt = MALLOC(sizeof(SHashElt));

  pHashElt->pFunctions = pFunctions;
  pHashElt->pElt = pElt;
  pHashElt->uRef = 0;

  return pHashElt;
}

// ----- hash_element_search -----------------------------------------
/**
 *
 *
 */
int hash_element_search(SPtrArray * aHashElts, void * pElt, 
			SHashFunctions * pFunctions)
{
  int iIndex;
  SHashElt * pHashElt = MALLOC(sizeof(SHashElt));

  pHashElt->pElt = pElt;
  pHashElt->pFunctions = pFunctions;
  if (ptr_array_sorted_find_index(aHashElts, &pHashElt, &iIndex) == -1)
    iIndex = -1;

  FREE(pHashElt);

  return iIndex;
}

// ----- hash_element_add --------------------------------------------
/**
 *
 *
 */
SHashElt * hash_element_add(SPtrArray * aHashElt, void * pElt, 
		      SHashFunctions * pFunctions)
{
    SHashElt * pHashElt = hash_element_init(pElt, pFunctions);
    ptr_array_add(aHashElt, &pHashElt);
    return pHashElt;
}

// ----- hash_init ---------------------------------------------------
/**
 *
 *
 */
SHash * hash_init(const uint32_t uHashSize, FHashEltCompare fEltCompare, 
		    FHashEltDestroy fEltDestroy, FHashCompute fHashCompute)
{
  SHash * pHash = MALLOC(sizeof(SHash));

  pHash->aHash = MALLOC(sizeof(uint32_t)*uHashSize);
  memset(pHash->aHash, 0, sizeof(uint32_t)*uHashSize);

  if (fHashCompute == NULL)
    LOG_WARNING("hash_init> Hash Key Computation function unitialized !");

  pHash->pFunctions = MALLOC(sizeof(SHashFunctions));
  pHash->pFunctions->fEltCompare = fEltCompare;
  pHash->pFunctions->fEltDestroy = fEltDestroy;
  pHash->pFunctions->fHashCompute = fHashCompute;

  pHash->uHashSize = uHashSize;

  return pHash;
}

// ----- hash_add ----------------------------------------------------
/**
 *
 * returns NULL if error else pointer to the Elt added
 */
int hash_add(SHash * pHash, void * pElt)
{
  uint32_t uHashKey = pHash->pFunctions->fHashCompute(pElt);
  int iIndex = 0;
  SHashElt * pHashElt;
  SPtrArray * aHashElt;

  if ( (uHashKey = pHash->pFunctions->fHashCompute(pElt)) >= pHash->uHashSize) {
    LOG_WARNING("hash_add> Key size too large.\n");
    return -1;
  }
  
  if ( (aHashElt = pHash->aHash[uHashKey]) == NULL)  
    aHashElt = pHash->aHash[uHashKey] = ptr_array_create(ARRAY_OPTION_UNIQUE|ARRAY_OPTION_SORTED,
							hash_element_compare,
							hash_element_destroy);

  if ( (iIndex = hash_element_search(aHashElt, pElt, pHash->pFunctions)) == -1) {
    pHashElt = hash_element_add(aHashElt, pElt, pHash->pFunctions);
  } else {
    pHashElt = aHashElt->data[iIndex];
  }

  hash_element_ref(pHashElt);

  return uHashKey;
}

// ----- hash_search -------------------------------------------------
/**
 *
 * returns NULL if no elt found else the pointer to this elt
 */
void * hash_search(SHash * pHash, void * pElt)
{
  int iIndex;
  SPtrArray * aHashElts;
  SHashElt * pHashEltSearched = NULL;
  uint32_t uHashKey = pHash->pFunctions->fHashCompute(pElt);

  if ( (aHashElts = pHash->aHash[uHashKey]) != NULL) {
    if ( (iIndex = hash_element_search(aHashElts, pElt, pHash->pFunctions)) != -1) 
      pHashEltSearched = aHashElts->data[iIndex];
  }
  return (pHashEltSearched == NULL) ? NULL : pHashEltSearched->pElt;
}

// ----- hash_del ----------------------------------------------------
/**
 *
 *  returns 0 if no elt deleted (unreferenced) else 1 if unreferenced 
 *  otherwise 2 if deleted
 */
int hash_del(SHash * pHash, void * pElt)
{
  uint32_t uHashKey = pHash->pFunctions->fHashCompute(pElt);
  SPtrArray * aHashElt;
  int iRet = 0, iIndex;
  
  if ( (aHashElt = pHash->aHash[uHashKey]) != NULL) {
    if ( (iIndex = hash_element_search(aHashElt, pElt, pHash->pFunctions)) != -1) {
      iRet = 1;
      if (hash_element_unref(&aHashElt->data[iIndex]) <= 0){
	iRet = 2;
	ptr_array_remove_at(aHashElt, iIndex);
      }
    }
  }
  if (iRet == 0)
    LOG_DEBUG("hash_del> No elt unreferenced.\n");

  return iRet;
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
