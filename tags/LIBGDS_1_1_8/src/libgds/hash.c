// ==================================================================
// @(#)hash.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 03/12/2004
// @lastdate 27/01/2005
// ==================================================================

/* This code implements a hash table structure. We can insert same 
 * elements as many as it is needed. It is managed with a reference 
 * counter.
 */

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
  uint32_t uKeyNonModulo;
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

// ----- hash_element_remove -----------------------------------------
/**
 *
 *
 */
void hash_element_remove(void * pHelt)
{
  SHashElt * pHashElt = *((SHashElt **)pHelt);

  if (pHashElt != NULL)
    FREE(pHashElt);
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
  FREE(pHashElt);
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
SHashElt * hash_element_init(void * pElt, uint32_t uKeyNonModulo, 
			      SHashFunctions * pFunctions)
{
  SHashElt * pHashElt = MALLOC(sizeof(SHashElt));

  pHashElt->pFunctions = pFunctions;
  pHashElt->pElt = pElt;
  pHashElt->uRef = 0;
  pHashElt->uKeyNonModulo = uKeyNonModulo;

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
		      uint32_t uKeyNonModulo, SHashFunctions * pFunctions)
{
    SHashElt * pHashElt = hash_element_init(pElt, uKeyNonModulo, pFunctions);
    ptr_array_add(aHashElt, &pHashElt);
    return pHashElt;
}

// ----- hash_init ---------------------------------------------------
/**
 *
 *
 */
SHash * hash_init(const uint32_t uHashSize, float fResizeThreshold, 
		    FHashEltCompare fEltCompare, 
		    FHashEltDestroy fEltDestroy, 
		    FHashCompute fHashCompute)
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
  pHash->fResizeThreshold = fResizeThreshold;
  pHash->uEltCount = 0;

  return pHash;
}


// ----- hash_rehash -------------------------------------------------
/**
 *
 *
 */
void hash_rehash(SHash * pHash)
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

}

// ----- hash_add ----------------------------------------------------
/**
 *
 * returns -1 if error else the key at which the elt has been added
 */
int hash_add(SHash * pHash, void * pElt)
{
  uint32_t uHashKey, uKeyNonModulo;
  int iIndex = 0;
  SHashElt * pHashElt;
  SPtrArray * aHashElt;

  uKeyNonModulo = pHash->pFunctions->fHashCompute(pElt);
  if ( (uHashKey = uKeyNonModulo%pHash->uHashSize) >= pHash->uHashSize) {
    LOG_WARNING("hash_add> Key size too large.\n");
    return -1;
  }
  
  if ( (aHashElt = pHash->aHash[uHashKey]) == NULL)  
    aHashElt = pHash->aHash[uHashKey] = ptr_array_create(ARRAY_OPTION_UNIQUE|ARRAY_OPTION_SORTED,
							hash_element_compare,
							hash_element_destroy);

  if ( (iIndex = hash_element_search(aHashElt, pElt, pHash->pFunctions)) == -1) {
    if (++pHash->uEltCount > (uint32_t)((float)pHash->uHashSize*pHash->fResizeThreshold)) {
      hash_rehash(pHash);
    }
    pHashElt = hash_element_add(aHashElt, pElt, uKeyNonModulo, pHash->pFunctions);
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
  uint32_t uHashKey = pHash->pFunctions->fHashCompute(pElt)%pHash->uHashSize;

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
  uint32_t uHashKey = pHash->pFunctions->fHashCompute(pElt)%pHash->uHashSize;
  SPtrArray * aHashElt;
  int iRet = 0, iIndex;
  
  if ( (aHashElt = pHash->aHash[uHashKey]) != NULL) {
    if ( (iIndex = hash_element_search(aHashElt, pElt, pHash->pFunctions)) != -1) {
      iRet = 1;
      if (hash_element_unref(&aHashElt->data[iIndex]) <= 0){
	iRet = 2;
	pHash->uEltCount--;
	ptr_array_remove_at(aHashElt, iIndex);
      }
    }
  }
  if (iRet == 0)
    LOG_DEBUG("hash_del> No elt unreferenced.\n");

  return iRet;
}

// ----- hash_for_each -----------------------------------------------
/**
 *
 *
 */
void hash_for_each(SHash * pHash, FHashForEach fHashForEach, 
		      void * pContext)
{
  uint32_t uHashKey, uArrayIter;
  SPtrArray * aHashElt;

  for (uHashKey = 0; uHashKey < pHash->uHashSize; uHashKey++) {
    if ( (aHashElt = pHash->aHash[uHashKey]) != NULL) {
      for (uArrayIter = 0; uArrayIter < ptr_array_length(aHashElt); uArrayIter++) {
	fHashForEach((*(SHashElt **)aHashElt->data[uArrayIter])->pElt, pContext);
      }
    }
  }
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
