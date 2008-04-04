// ==================================================================
// @(#)array.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 10/04/2003
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <libgds/array.h>
#include <libgds/enumerator.h>
#include <libgds/memory.h>

#define _array_elt_pos(A,i) (((char *) A->data)+ \
			    (i)*((SRealArray *) A)->uEltSize)
#define _array_size(A) ((SRealArray *) A)->uEltSize* \
                       ((SRealArray *) A)->uLength

typedef struct {
  FArrayCompare compare;
  FArrayDestroy destroy;
} _array_ops_t;

typedef struct {
  uint8_t      ** data;
  unsigned int    uLength;
  unsigned int    uEltSize;
  uint8_t         uOptions;
  _array_ops_t    ops;
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
  pRealArray->ops.compare= fCompare;
  pRealArray->ops.destroy= fDestroy;
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

  pRealArray->ops.destroy = fDestroy;
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
      if ((*ppRealArray)->ops.destroy != NULL)
	for (uIndex= 0; uIndex < (*ppRealArray)->uLength; uIndex++)
	  (*ppRealArray)->ops.destroy(_array_elt_pos((*ppRealArray), uIndex));
      FREE((*ppRealArray)->data);
    }
    FREE(*ppRealArray);
    *ppRealArray= NULL;
  }
}

// ----- _array_resize_if_required ----------------------------------
/**
 * Change the size of an array. Re-allocate memory accordingly.
 */
static void _array_resize_if_required(SArray * pArray,
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
 * Return the length of the array.
 */
unsigned int _array_length(SArray * pArray)
{
  return ((SRealArray *) pArray)->uLength;
}

// ----- _array_set_length ------------------------------------------
/**
 * Set the length of an array. If the new size is smaller than the
 * original size, data will be lost.
 */
void _array_set_length(SArray * pArray, unsigned int uNewLength)
{
  _array_resize_if_required(pArray, uNewLength);
}

// ----- array_set_at -----------------------------------------------
/**
 * Set the value of the element at the given index in the array.
 *
 * RETURNS:
 *   >=0 (index) in case of success
 *    -1 in case of failure (index >= length)
 */
int _array_set_at(SArray * pArray, unsigned int uIndex, void * pData)
{
  if (uIndex >= ((SRealArray *) pArray)->uLength)
    return -1;
  memcpy(_array_elt_pos(pArray, uIndex), pData,
  	 ((SRealArray *) pArray)->uEltSize);
  return uIndex;
}

// ----- array_get_at -----------------------------------------------
/**
 * Return the value at the given index in the array.
 *
 * RETURNS:
 *    0 in case of success
 *   -1 in case of failure (index >= length)
 */
int _array_get_at(SArray * pArray, unsigned int uIndex, void * pData)
{
  if (uIndex >= ((SRealArray *) pArray)->uLength)
    return -1;

  memcpy(pData, _array_elt_pos(pArray, uIndex),
	 ((SRealArray *) pArray)->uEltSize);
  return 0;
}

// ----- _array_sorted_find_index -----------------------------------
/**
 * Find the index of an element in a sorted array (using the compare
 * function)
 *
 * RETURNS:
 *    0 in case of success (the index of the element is returned)
 *   -1 in case of failure (the index where this element would be
 *                          placed is returned)
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
      (((SRealArray *) pArray)->ops.compare(_array_elt_pos(pArray, uPos),
					    pData,
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
 * Insert an element in the array at the specified index.
 *
 * RETURNS:
 *   >= 0 in case of success
 *     -1 in case of failure
 *          (index >= length)
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
 * Add an element to the array.
 * 1). If the array is sorted, the element is inserted according to
 *     the ordering defined by the compare function.
 * 2). If the array is not sorted, the element is inserted at the end
 *     of the array
 *
 * RETURNS:
 *   >= 0 in case of success
 *     -1 in case of failure
 *          if (sorted && ARRAY_OPTION_UNIQUE && value exists)
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
 * Append en element at the end of the array. Note that this function
 * should not be used with sorted arrays.
 *
 * RETURNS:
 *   >=0 insertion index
 */
int _array_append(SArray * pArray, void * pData)
{
  assert((((SRealArray *) pArray)->uOptions & ARRAY_OPTION_SORTED) == 0);

  _array_resize_if_required(pArray, ((SRealArray *) pArray)->uLength+1);

  _array_set_at(pArray, ((SRealArray *) pArray)->uLength-1, pData);
  return ((SRealArray *) pArray)->uLength-1;
}

// ----- _array_for_each --------------------------------------------
/**
 * Execute the given callback function for each element in the array.
 * The callback function must return 0 in case of success and !=0 in
 * case of failure. In the later case, the array traversal will stop
 * and the error code of the callback is returned.
 *
 * RETURNS:
 *     0 in case of success
 *   !=0 in case of failure
 */
int _array_for_each(SArray * pArray, FArrayForEach fForEach,
		    void * pContext)
{
  unsigned int uIndex;
  int iResult;
  
  for (uIndex= 0; uIndex < _array_length(pArray); uIndex++) {
    iResult= fForEach(_array_elt_pos(pArray, uIndex), pContext);
    if (iResult != 0)
      return iResult;
  }
  return 0;
}

// ----- _array_copy ------------------------------------------------
/**
 * Make a copy of an entire array.
 *
 * RETURNS:
 *   a pointer to the copy
 */
SArray * _array_copy(SArray * pArray)
{
  SArray * pNewArray= _array_create(((SRealArray *) pArray)->uEltSize,
				    ((SRealArray *) pArray)->uOptions,
				    ((SRealArray *) pArray)->ops.compare,
				    ((SRealArray *) pArray)->ops.destroy);
  _array_set_length(pNewArray, ((SRealArray *)pArray)->uLength);
  memcpy(pNewArray->data, pArray->data, _array_size(pArray));
  return pNewArray;
}

// ----- _array_remove_at -------------------------------------------
/**
 * Remove the element at the given index in the array.
 *
 * RETURNS:
 *    0 un case of success
 *   -1 in case index is not valid
 */
int _array_remove_at(SArray * pArray, unsigned int uIndex)
{
  SRealArray * pRealArray= (SRealArray *) pArray;
  unsigned int uOffset;

  if (uIndex < pRealArray->uLength) {

    // Free item at given position if required
    if (pRealArray->ops.destroy != NULL)
      pRealArray->ops.destroy(_array_elt_pos(pRealArray, uIndex));

    // Since (uIndex >= 0), then (pRealArray->uLength >= 1) and then
    // there is no problem with the unsigned variable uOffset.
    for (uOffset= uIndex; uOffset < pRealArray->uLength-1; uOffset++) {
      memcpy(_array_elt_pos(pArray, uOffset),
	     _array_elt_pos(pArray, uOffset+1),
	     pRealArray->uEltSize);
    }
    _array_resize_if_required(pArray, pRealArray->uLength-1);
  } else {
    return -1;
  }
  return 0;
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
		  ((SRealArray *) pArray)->ops.compare,
		  ((SRealArray *) pArray)->ops.destroy);
  pSubArray->uLength= iLast-iFirst+1;
  pSubArray->data= (uint8_t **) MALLOC(pSubArray->uEltSize*pSubArray->uLength);
  memcpy(pSubArray->data, _array_elt_pos(pArray, iFirst),
	 pSubArray->uEltSize*pSubArray->uLength);
  return (SArray *) pSubArray;
}

// ----- _array_add_array -------------------------------------------
/**
 * Add a whole array to another array.
 */
void _array_add_array(SArray * pArray, SArray * pSrcArray)
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
  unsigned int index, index2;
  void * pTemp= MALLOC(((SRealArray *) pArray)->uEltSize);

  for (index= 0; index < _array_length(pArray); index++)
    for (index2= index; index2 > 0; index2--)
      if (fCompare(_array_elt_pos(pArray, index2-1),
		   _array_elt_pos(pArray, index2),
		   ((SRealArray *) pArray)->uEltSize) > 0) {
	_array_elt_copy_from(pArray, pTemp, index2);
	_array_elt_copy(pArray, index2, index2-1);
	_array_elt_copy_to(pArray, index2-1, pTemp);
      }

  FREE(pTemp);
  ((SRealArray*) pArray)->uOptions|= ARRAY_OPTION_SORTED;
  ((SRealArray*) pArray)->ops.compare= fCompare;
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
  // NOT YET IMPLEMENTED
  return -1;
}


/////////////////////////////////////////////////////////////////////
//
// ENUMERATION
//
/////////////////////////////////////////////////////////////////////

typedef struct {
  unsigned int   index;
  SArray       * array;
} _enum_ctx_t;

// -----[ _enum_has_next ]-------------------------------------------
static int _enum_has_next(void * ctx)
{
  _enum_ctx_t * enum_ctx= (_enum_ctx_t *) ctx;
  return (enum_ctx->index < _array_length(enum_ctx->array));
}

// -----[ _enum_get_next ]-------------------------------------------
static void * _enum_get_next(void * ctx)
{
  _enum_ctx_t * enum_ctx= (_enum_ctx_t *) ctx;
  return _array_elt_pos(enum_ctx->array, enum_ctx->index++);
}

// -----[ _enum_destroy ]--------------------------------------------
static void _enum_destroy(void * ctx)
{
  _enum_ctx_t * enum_ctx= (_enum_ctx_t *) ctx;
  FREE(enum_ctx);
}

// -----[ _array_get_enum ]------------------------------------------
enum_t * _array_get_enum(SArray * array)
{
  _enum_ctx_t * ctx= (_enum_ctx_t *) MALLOC(sizeof(_enum_ctx_t));
  ctx->array= array;
  ctx->index= 0;
  return enum_create(ctx,
		     _enum_has_next,
		     _enum_get_next,
		     _enum_destroy);
}


