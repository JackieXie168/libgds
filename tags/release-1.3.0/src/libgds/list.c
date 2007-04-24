// ==================================================================
// @(#)list.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Sebastien Tandel (sta@info.ucl.ac.be)
// @date 23/11/2002
// @lastdate 18/03/2007
// ==================================================================
// Warning: performance of 'list_add' are poor (due to realloc +
// memmove) but memory usage and research are optimal !!!
// 
// 13/06/2003:
//   - added the possibility to resize the list by uStepResize. it
//     improves the performance by reducing the number of realloc
//     done but the number of calls to memmove() remains the same !
// 09/12/2004: 
//   - bug fixes (?)
//   - This code is only used to make the garbage collector for
//     memory_debug.c. For this reason, it doesn't use the MALLOC,
//     FREE and REALLOC wrappers but directly uses malloc, free and
//     realloc. If you need to use a list or an array, choose the
//     array implementation instead.
// 18/03/2007:
//   - add assert(ptr != NULL) for each call to malloc() and realloc().
//   - _list_resize() is now a static function
//
// TODO:
//   - current code of list_add() is very inefficient. The calls to
//     list_find_index() and  list_insert_index() should be merged
//     in order to perform a single lookup !!!
//   - rename this collection. The name "list" is not appropriate,
//     it is rather a sorted list
//   - since this data structure is only used internally, its's API
//     (header files) should not be disclosed.
// ==================================================================


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgds/list.h>

// -----[ _list_compare ]--------------------------------------------
/**
 *
 */
static inline int _list_compare(void * pItem1, void * pItem2)
{
  if (pItem1 == pItem2)
    return 0;
  else if (pItem1 < pItem2)
    return -1;
  else
    return 1;
}

// ----- list_create ------------------------------------------------
/**
 *
 */
SList * list_create(FListCompare fCompare,
		    FListDestroy fDestroy, unsigned int uStepResize)
{
  SList * pList= (SList *) malloc(sizeof(SList));
  assert(pList != NULL);
  pList->iSize= 0;
  pList->uNbrElt = 0;
  if (uStepResize == 0) 
    uStepResize = 1;
  pList->uStepResize = uStepResize;
  pList->fCompare= fCompare;
  pList->fDestroy= fDestroy;
  pList->ppItems= NULL;
  return pList;
}

// ----- list_destroy -----------------------------------------------
/**
 *
 */
void list_destroy(SList ** ppList)
{
  int iIndex;

  if (*ppList != NULL) {
    if ((*ppList)->ppItems != NULL) {
      if ((*ppList)->fDestroy != NULL)
	for (iIndex= 0; iIndex < (*ppList)->uNbrElt; iIndex++)
	  (*ppList)->fDestroy(&(*ppList)->ppItems[iIndex]);
      free((*ppList)->ppItems);
      (*ppList)->ppItems= NULL;
    }
    free(*ppList);
    *ppList= NULL;
  }
}

// ----- list_find_index --------------------------------------------
/**
 * ITEM found => 0, INDEX of item
 * ITEM not found => -1, INDEX where insertion must occur
 */
int list_find_index(SList * pList, void * pItem, int * piIndex)
{
  int iOffset= 0;
  int iSize= pList->uNbrElt;
  int iPos= iSize/2;
  int iCompareResult;

  while (iSize > 0) {
    iCompareResult= ((pList->fCompare != NULL)?
		     pList->fCompare(pList->ppItems[iPos], pItem):
		     _list_compare(pList->ppItems[iPos], pItem));
    if (!iCompareResult) {
      *piIndex= iPos;
      return 0;
    } else if (iCompareResult > 0) {
      if (iPos > iOffset) {
	iSize= iPos-iOffset;
	iPos= iOffset+iSize/2;
      } else
	break;
    } else {
      if (iOffset+iSize-iPos > 0) {
	iSize= iOffset+iSize-iPos-1;
	iOffset= iPos+1;
	iPos= iOffset+iSize/2;
      } else
	break;
    }
  }
  *piIndex= iPos;
  return -1;
}

// -----[ _list_resized_if_required ]--------------------------------
/**
 *
 */
static void _list_resize(SList * pList)
{
  if (pList->ppItems != NULL) {
    if (pList->iSize == 0) {
      free(pList->ppItems);
      pList->ppItems = NULL;
    } else {
      pList->ppItems= realloc(pList->ppItems, sizeof(void *)*pList->iSize);
      assert(pList->ppItems != NULL);
    }
  } else {
    pList->ppItems= malloc(sizeof(void *)*pList->iSize);
    assert(pList->ppItems != NULL);
  }
}

// ----- list_insert_index ------------------------------------------
/**
 *
 */
int list_insert_index(SList * pList, int iIndex, void * pItem)
{
  if ((iIndex < 0) || (iIndex > pList->uNbrElt))
    return -1;
  
  pList->uNbrElt++;
  if (pList->uNbrElt >= pList->iSize) {
    pList->iSize += pList->uStepResize;
    _list_resize(pList);
  }
  
  if (iIndex < pList->uNbrElt - 1) {
    memmove(&pList->ppItems[iIndex+1], &pList->ppItems[iIndex],
	    sizeof(void *)*(pList->uNbrElt-iIndex-1));
  }
  
  pList->ppItems[iIndex]= pItem;
  return 0;
}

// ----- list_get_nbr_element ---------------------------------------
/**
 *
 */
int list_get_nbr_element(SList * pList)
{
  return pList->uNbrElt;
}

// ----- list_get_index ---------------------------------------------
/*
 *
 */
void * list_get_index(SList * pList, int iIndex)
{
  if (iIndex >= pList->uNbrElt)
    return NULL;
  
  return pList->ppItems[iIndex];
}

// ----- list_add ---------------------------------------------------
/**
 * TODO: current code is very inefficient. list_find_index() and
 * list_insert_index() should be merged in order to perform a single
 * lookup !!!
 */
int list_add(SList * pList, void * pItem)
{
  int iIndex;

  if (list_find_index(pList, pItem, &iIndex) < 0)
    return list_insert_index(pList, iIndex, pItem);
  return -1; // Item already exists
}

// ----- list_delete ------------------------------------------------
/*
 *
 */
int list_delete(SList * pList, int iIndex)
{
  if (iIndex > pList->uNbrElt)
    return -1;
  
  if (pList->fDestroy != NULL)
    pList->fDestroy(&pList->ppItems[iIndex]);
  
  
  if (pList->uNbrElt - (iIndex+1) != 0) {
    /*fprintf(stderr, "@%d: memcpy %p %p %p %d\n", iIndex,
      pList->ppItems[iIndex], pList->ppItems[iIndex+1], pList->ppItems[iIndex+pList->uNbrElt-iIndex-1], pList->uNbrElt-iIndex-1);*/
    memmove(&(pList->ppItems[iIndex]), &(pList->ppItems[iIndex+1]), sizeof(void *)*(pList->uNbrElt - (iIndex+1)));
  }
  
  pList->uNbrElt--;
  if (pList->uStepResize <= pList->iSize - pList->uNbrElt) {
    pList->iSize -= pList->uStepResize;
    _list_resize(pList);
  }
  return 0;
}

// ----- list_replace -----------------------------------------------
/**
 *
 */
int list_replace(SList * pList, int iIndex, void * pItem)
{
  if (iIndex >= pList->uNbrElt)
    return -1;

  if (pList->fDestroy != NULL)
    pList->fDestroy(&pList->ppItems[iIndex]);
  pList->ppItems[iIndex]= pItem;
  return 0;
}

// ----- list_for_each ----------------------------------------------
/**
 *
 */
void list_for_each(SList * pList, FListForEach fForEach, void * pContext)
{
  int iIndex;

  for (iIndex= 0; iIndex < pList->uNbrElt; iIndex++)
    fForEach(pList->ppItems[iIndex], pContext);
}

// ----- list_copy --------------------------------------------------
/**
 * We have to copy the empty elements
 */
SList * list_copy(SList * pList, FListCopyItem fCopyItem)
{
  int iIndex;

  SList * pNewList= list_create(pList->fCompare, pList->fDestroy,
				pList->uStepResize);
  pNewList->iSize= pList->iSize;
  pNewList->uNbrElt= pList->uNbrElt;
	
  pNewList->ppItems= malloc(sizeof(void *)*pNewList->iSize);
  assert(pNewList->ppItems != NULL);
  if (fCopyItem == NULL) {
    memcpy(pNewList->ppItems, pList->ppItems,
	   sizeof(void *)*pNewList->uNbrElt);
  } else {
    for (iIndex= 0; iIndex < pNewList->uNbrElt; iIndex++)
      pNewList->ppItems[iIndex]= fCopyItem(pList->ppItems[iIndex]);
  }
  return pNewList;
}

