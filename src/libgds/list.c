// ==================================================================
// @(#)list.c
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be), Sebastien Tandel
// @date 23/11/2002
// @lastdate 13/06/2003
// ==================================================================
// Warning: performance of 'list_add' are poor (due to realloc +
// memmove) but memory usage and research are optimal !!!
// 
// 13/06/2003 : added the possibility to resize the list by uStepResize.
// it improves the performance by reducing the number of realloc done 
// but the number of memmove remains the same !

#include <stdlib.h>
#include <string.h>

#include <libgds/list.h>
#include <libgds/memory.h>

// ----- list_compare -----------------------------------------------
/**
 *
 */
int list_compare(void * pItem1, void * pItem2)
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
  SList * pList= (SList *) MALLOC(sizeof(SList));
  pList->iSize= 0;
	pList->uNbrElt = 0;
	if (pList->uStepResize == 0) 
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
      FREE((*ppList)->ppItems);
      (*ppList)->ppItems= NULL;
    }
    FREE(*ppList);
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
		     list_compare(pList->ppItems[iPos], pItem));
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

// ----- list_resized_if_required -----------------------------------
/**
 *
 */
void list_resize(SList * pList)
{
	if (pList->ppItems != NULL)
		if (pList->iSize == 0) {
			FREE(pList->ppItems);
			pList->ppItems = NULL;
		}
		else
			pList->ppItems= REALLOC(pList->ppItems, sizeof(void *)*pList->iSize);
	else
		pList->ppItems= MALLOC(sizeof(void *)*pList->iSize);
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
		list_resize(pList);
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
 *
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
	
	memcpy(pList->ppItems[iIndex], pList->ppItems[iIndex+1], sizeof(void *)*(pList->uNbrElt - (iIndex+1)));
	
	if (pList->uStepResize <= pList->iSize - --(pList->uNbrElt)) {
		pList->iSize -= pList->uStepResize;
		list_resize(pList);
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

  for (iIndex= 0; iIndex < pList->iSize; iIndex++)
    fForEach(pList->ppItems[iIndex], pContext);
}

// ----- list_copy --------------------------------------------------
/**
 * We have to copy the empty elements
 */
SList * list_copy(SList * pList, FListCopyItem fCopyItem)
{
  int iIndex;

  SList * pNewList= list_create(pList->fCompare, pList->fDestroy, pList->uStepResize);
  pNewList->iSize= pList->iSize;
	pNewList->uNbrElt = pList->uNbrElt;
	
  pNewList->ppItems= MALLOC(sizeof(void *)*pNewList->iSize);
  if (fCopyItem == NULL)
    memcpy(pNewList->ppItems, pList->ppItems, sizeof(void *)*pNewList->uNbrElt);
  else
    for (iIndex= 0; iIndex < pNewList->uNbrElt; iIndex++)
      pNewList->ppItems[iIndex]= fCopyItem(pList->ppItems[iIndex]);
  return pNewList;
}

