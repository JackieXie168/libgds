// ==================================================================
// @(#)array.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 10/04/2003
// @lastdate 27/01/2005
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <libgds/array.h>
#include <libgds/memory.h>

#define _array_elt_pos(A,i) (((char *) A->data)+ \
			    (i)*((SRealArray *) A)->uEltSize)
#define _array_size(A) ((SRealArray *) A)->uEltSize* \
                       ((SRealArray *) A)->uLength

typedef struct {
  uint8_t ** data;
  unsigned int uLength;
  unsigned int uEltSize;
  uint8_t uOptions;
  FArrayCompare fCompare;
  FArrayDestroy fDestroy;
} SRealArray;

// ----- _array_compare ---------------------------------------------
/**
 * \brief Compare two elements of an array.
 *
 * @param pItem1 the first element.
 * @param pItem2 the second element.
 * @param uEltSize the elements'size.
 */
int _array_compare(void * pItem1, void * pItem2,
		   unsigned int uEltSize)
{
  return memcmp(pItem1, pItem2, uEltSize);
}

// ----- _array_create ----------------------------------------------
/**
 *
 */
SArray * _array_create(unsigned int uEltSize, uint8_t uOptions,
		       FArrayCompare fCompare,
		       FArrayDestroy fDestroy)
{
  SRealArray * pRealArray= (SRealArray *) MALLOC(sizeof(SRealArray));
  pRealArray->uLength= 0;
  pRealArray->uEltSize= uEltSize;
  pRealArray->data= NULL;
  pRealArray->uOptions= uOptions;
  pRealArray->fCompare= fCompare;
  pRealArray->fDestroy= fDestroy;
  return (SArray *) pRealArray;
}

// ----- _array_set_fdestroy -----------------------------------------
/**
 *
 *
 */
void _array_set_fdestroy(SArray * pArray, FArrayDestroy fDestroy)
{
  SRealArray * pRealArray = (SRealArray *)pArray;

  pRealArray->fDestroy = fDestroy;
}

// ----- _array_destroy ----------------------------------------------
/**
 *
 */
void _array_destroy(SArray ** ppArray)
{
  SRealArray ** ppRealArray=
    (SRealArray **) ppArray;
  unsigned int uIndex;

  if (*ppRealArray != NULL) {
    if ((*ppRealArray)->uLength > 0) {
      if ((*ppRealArray)->fDestroy != NULL)
	for (uIndex= 0; uIndex < (*ppRealArray)->uLength; uIndex++)
	  (*ppRealArray)->fDestroy(_array_elt_pos((*ppRealArray), uIndex));
      FREE((*ppRealArray)->data);
    }
    FREE(*ppRealArray);
    *ppRealArray= NULL;
  }
}

// ----- _array_resize_if_required ----------------------------------
/**
 *
 */
void _array_resize_if_required(SArray * pArray,
			       unsigned int uNewLength)
{
  SRealArray * pRealArray= (SRealArray *) pArray;

  if (uNewLength != pRealArray->uLength) {
    if (pRealArray->uLength == 0) {
      pRealArray->data=
	(uint8_t **) MALLOC(uNewLength*pRealArray->uEltSize);
    } else if (uNewLength == 0) {
      FREE(pRealArray->data);
      pRealArray->data= NULL;
    } else {
      pRealArray->data=
	(uint8_t **) REALLOC(pRealArray->data,
			     uNewLength*pRealArray->uEltSize);
    }
    pRealArray->uLength= uNewLength;
  }
}

// ----- _array_length -----------------------------------------------
/**
 *
 */
int _array_length(SArray * pArray)
{
  return ((SRealArray *) pArray)->uLength;
}

// ----- _array_set_length ------------------------------------------
/**
 *
 */
void _array_set_length(SArray * pArray, unsigned int uNewLength)
{
  _array_resize_if_required(pArray, uNewLength);
}

// ----- array_set_at -----------------------------------------------
/**
 *
 */
int _array_set_at(SArray * pArray, unsigned int uIndex, void * pData)
{
  if (uIndex >= ((SRealArray *) pArray)->uLength)
    return -1;
  memcpy(_array_elt_pos(pArray, uIndex), pData,
  	 ((SRealArray *) pArray)->uEltSize);
  return 0;
}

// ----- array_get_at -----------------------------------------------
/**
 *
 */
void _array_get_at(SArray * pArray, unsigned int uIndex, void * pData)
{
  memcpy(pData, _array_elt_pos(pArray, uIndex),
	 ((SRealArray *) pArray)->uEltSize);
}

// ----- _array_sorted_find_index -----------------------------------
/**
 * ITEM found => 0, INDEX of item
 * ITEM not found => -1, INDEX where insertion must occur
 */
int _array_sorted_find_index(SArray * pArray, void * pData,
			     unsigned int * puIndex)
{
  unsigned int uOffset= 0;
  unsigned int uSize= ((SRealArray *) pArray)->uLength;
  unsigned int uPos= uSize/2;
  int iCompareResult;

  while (uSize > 0) {
    iCompareResult=
      (((SRealArray *) pArray)->fCompare(_array_elt_pos(pArray, uPos), pData,
					 ((SRealArray *) pArray)->uEltSize));
    if (!iCompareResult) {
      *puIndex= uPos;
      return 0;
    } else if (iCompareResult > 0) {
      if (uPos > uOffset) {
	uSize= uPos-uOffset;
	uPos= uOffset+uSize/2;
      } else
	break;
    } else {
      if (uOffset+uSize-uPos > 0) {
	uSize= uOffset+uSize-uPos-1;
	uOffset= uPos+1;
	uPos= uOffset+uSize/2;
      } else
	break;
    }
  }
  *puIndex= uPos;
  return -1;
}

// ----- _array_insert_at -------------------------------------------
/**
 * condition: (0 <= index < length)
 */
int _array_insert_at(SArray * pArray, unsigned int uIndex, void * pData)
{
  unsigned int uOffset;
  SRealArray * pRealArray= (SRealArray *) pArray;

  if (uIndex > ((SRealArray *) pArray)->uLength)
    return -1;
  _array_resize_if_required(pArray, ((SRealArray *) pArray)->uLength+1);
  for (uOffset= pRealArray->uLength-1; uOffset > uIndex; uOffset--) {
    memcpy(_array_elt_pos(pArray, uOffset),
	   _array_elt_pos(pArray, uOffset-1),
	   pRealArray->uEltSize);
  }
  return _array_set_at(pArray, uIndex, pData);
}

// ----- _array_add -------------------------------------------------
/**
 *
 */
int _array_add(SArray * pArray, void * pData)
{
  unsigned int uIndex;

  if (((SRealArray *) pArray)->uOptions & ARRAY_OPTION_SORTED) {
    if (_array_sorted_find_index(pArray, pData, &uIndex) < 0) {
      return _array_insert_at(pArray, uIndex, pData);
    } else {
      if (((SRealArray *) pArray)->uOptions & ARRAY_OPTION_UNIQUE)
	return -1;
      else
	return _array_set_at(pArray, uIndex, pData);
    }
  } else
    return _array_append(pArray, pData);
}

// ----- array_append -----------------------------------------------
/**
 *
 */
int _array_append(SArray * pArray, void * pData)
{
  _array_resize_if_required(pArray, ((SRealArray *) pArray)->uLength+1);

  _array_set_at(pArray, ((SRealArray *) pArray)->uLength-1, pData);
  return ((SRealArray *) pArray)->uLength-1;
}

// ----- _array_copy ------------------------------------------------
/**
 *
 */
SArray * _array_copy(SArray * pArray)
{
  SArray * pNewArray= _array_create(((SRealArray *) pArray)->uEltSize,
				    ((SRealArray *) pArray)->uOptions,
				    ((SRealArray *) pArray)->fCompare,
				    ((SRealArray *) pArray)->fDestroy);
  _array_set_length(pNewArray, ((SRealArray *)pArray)->uLength);
  memcpy(pNewArray->data, pArray->data, _array_size(pArray));
  return pNewArray;
}

// ----- _array_remove_at -------------------------------------------
/**
 *
 */
void _array_remove_at(SArray * pArray, unsigned int uIndex)
{
  SRealArray * pRealArray= (SRealArray *) pArray;
  unsigned int uOffset;

  // Free item at given position if required
  if (pRealArray->fDestroy != NULL)
    pRealArray->fDestroy(_array_elt_pos(pRealArray, uIndex));

  if (uIndex < pRealArray->uLength) {
    // Since (uIndex >= 0), then (pRealArray->uLength >= 1) and then
    // there is no problem with the unsigned variable uOffset.
    for (uOffset= uIndex; uOffset < pRealArray->uLength-1; uOffset++) {
      memcpy(_array_elt_pos(pArray, uOffset),
	     _array_elt_pos(pArray, uOffset+1),
	     pRealArray->uEltSize);
    }
    _array_resize_if_required(pArray, pRealArray->uLength-1);
  }
}

// ----- _array_sub -------------------------------------------------
/**
 * Extract a sub-array from the given array.
 *
 * PRECONDITION: (iFirst <= iLast) AND (iLast < length(ARRAY))
 */
SArray * _array_sub(SArray * pArray, unsigned int iFirst, unsigned int iLast)
{
  SRealArray * pSubArray;
  assert((iFirst <= iLast) && (iLast < _array_length(pArray)));

  pSubArray= (SRealArray *)
    _array_create(((SRealArray *) pArray)->uEltSize,
		  ((SRealArray *) pArray)->uOptions,
		  ((SRealArray *) pArray)->fCompare,
		  ((SRealArray *) pArray)->fDestroy);
  pSubArray->uLength= iLast-iFirst+1;
  pSubArray->data= (uint8_t **) MALLOC(pSubArray->uEltSize*pSubArray->uLength);
  memcpy(pSubArray->data, _array_elt_pos(pArray, iFirst),
	 pSubArray->uEltSize*pSubArray->uLength);
  return (SArray *) pSubArray;
}

// ----- _array_add_all ---------------------------------------------
/**
 *
 */
void _array_add_all(SArray * pArray, SArray * pSrcArray)
{
  unsigned int uLength= ((SRealArray *) pArray)->uLength;

  assert(((SRealArray *) pArray)->uEltSize ==
	 ((SRealArray *) pSrcArray)->uEltSize);
  _array_resize_if_required(pArray,
			    uLength+((SRealArray *) pSrcArray)->uLength);
  memcpy(_array_elt_pos(pArray, uLength),
	 _array_elt_pos(pSrcArray, 0),
	 _array_size(pSrcArray));
}

// ----- _array_trim ------------------------------------------------
/**
 *
 */
void _array_trim(SArray * pArray, unsigned uMaxLength)
{
  assert(uMaxLength <= ((SRealArray *) pArray)->uLength);
  _array_resize_if_required(pArray, uMaxLength);
}



#define _array_elt_copy_to(A, i, d) memcpy(_array_elt_pos(A, i), d, \
                                           ((SRealArray *) A)->uEltSize)
#define _array_elt_copy_from(A, d, i) memcpy(d, _array_elt_pos(A, i), \
                                             ((SRealArray *) A)->uEltSize)
#define _array_elt_copy(A, i, j) memcpy(_array_elt_pos(A, i), \
                                        _array_elt_pos(A, j), \
                                        ((SRealArray *) A)->uEltSize)

// ----- _array_sort ------------------------------------------------
/**
 * Simple selection-sort.
 */
int _array_sort(SArray * pArray, FArrayCompare fCompare)
{
  int iIndex, iIndex2;
  void * pTemp= MALLOC(((SRealArray *) pArray)->uEltSize);

  for (iIndex= 0; iIndex < _array_length(pArray); iIndex++)
    for (iIndex2= iIndex; iIndex2 > 0; iIndex2--)
      if (fCompare(_array_elt_pos(pArray, iIndex2-1),
		   _array_elt_pos(pArray, iIndex2),
		   ((SRealArray *) pArray)->uEltSize) > 0) {
	_array_elt_copy_from(pArray, pTemp, iIndex2);
	_array_elt_copy(pArray, iIndex2, iIndex2-1);
	_array_elt_copy_to(pArray, iIndex2-1, pTemp);
      }

  FREE(pTemp);
  return 0;
}

// ----- _array_quicksort -------------------------------------------
/**
 * Nonrecursive quicksort implementation.
 *
 * Note:
 *   - quicksort is not stable (does not preserve previous order)
 *   - complexity of quicksort is O(N.log(N))
 *   - the stack depth should remain under log(N) thanks to pushing
 *     the largest subfiles first on the stack
 */
int _array_quicksort(SArray * pArray, FArrayCompare fCompare)
{
  // NOT YET IMPLEMENTED (pas le temps maintenant)
  return 0;
}

// ----- uint16_array_destroy ---------------------------------------
/**
 *
 */
void uint16_array_destroy(SUInt16Array ** ppArray)
{
  _array_destroy((SArray **) ppArray);
}


// ----- ptr_array_destroy ------------------------------------------
/**
 *
 */
void ptr_array_destroy(SPtrArray ** ppArray)
{
  _array_destroy((SArray **) ppArray);
}
