// ==================================================================
// @(#)sequence.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 23/11/2002
// @lastdate 27/01/2005
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgds/memory.h>
#include <libgds/sequence.h>

unsigned long sequence_create_count= 0;
unsigned long sequence_copy_count= 0;
unsigned long sequence_destroy_count= 0;

// ----- sequence_create --------------------------------------------
/**
 *
 */
SSequence * sequence_create(FSeqCompare fCompare,
			    FSeqDestroy fDestroy)
{
  SSequence * pSequence= (SSequence *) MALLOC(sizeof(SSequence));
  sequence_create_count++;
  pSequence->iSize= 0;
  pSequence->fCompare= fCompare;
  pSequence->fDestroy= fDestroy;
  pSequence->ppItems= NULL;
  return pSequence;
}

// ----- sequence_destroy -------------------------------------------
/**
 *
 */
void sequence_destroy(SSequence ** ppSequence)
{
  int iIndex;

  if (*ppSequence != NULL) {
    sequence_destroy_count++;
    if ((*ppSequence)->ppItems != NULL) {
      if ((*ppSequence)->fDestroy != NULL)
	for (iIndex= 0; iIndex < (*ppSequence)->iSize; iIndex++)
	  (*ppSequence)->fDestroy(&(*ppSequence)->ppItems[iIndex]);
      FREE((*ppSequence)->ppItems);
      (*ppSequence)->ppItems= NULL;
    }
    FREE(*ppSequence);
    *ppSequence= NULL;
  }
}

// ----- sequence_find_index ----------------------------------------
/**
 * ITEM found => INDEX of item (>= 0)
 * ITEM not found => -1
 */
int sequence_find_index(SSequence * pSequence, void * pItem)
{
  int iIndex= 0;
  int iCompareResult;

  while (iIndex < pSequence->iSize) {
    iCompareResult= ((pSequence->fCompare != NULL)?
      pSequence->fCompare(pSequence->ppItems[iIndex], pItem):
      (pSequence->ppItems[iIndex] == pItem));
    if (iCompareResult)
      return iIndex;
    iIndex++;
  }
  return -1;
}

// ----- sequence_insert_at -----------------------------------------
/**
 *
 */
int sequence_insert_at(SSequence * pSequence, int iIndex, void * pItem)
{
  if ((iIndex < 0) || (iIndex > pSequence->iSize))
    return -1;
  pSequence->iSize++;
  if (pSequence->ppItems != NULL) {
    pSequence->ppItems= REALLOC(pSequence->ppItems,
			    sizeof(void *)*pSequence->iSize);
    memmove(&pSequence->ppItems[iIndex+1], &pSequence->ppItems[iIndex],
	    sizeof(void *)*(pSequence->iSize-iIndex-1));
  } else
    pSequence->ppItems= MALLOC(sizeof(void *)*pSequence->iSize);
  pSequence->ppItems[iIndex]= pItem;
  return 0;
}

// ----- sequence_add -----------------------------------------------
/**
 *
 */
int sequence_add(SSequence * pSequence, void * pItem)
{
  return sequence_insert_at(pSequence, pSequence->iSize, pItem);
}

// ----- sequence_remove --------------------------------------------
/**
 *
 */
int sequence_remove(SSequence * pSequence, void * pItem)
{
  int iIndex;

  iIndex= sequence_find_index(pSequence, pItem);
  if (iIndex > 0)
    return sequence_remove_at(pSequence, iIndex);
  return 0;
}

// ----- sequence_remove_at -----------------------------------------
/**
 *
 */
int sequence_remove_at(SSequence * pSequence, int iIndex)
{
  if ((iIndex < 0) || (iIndex >= pSequence->iSize))
    return -1;

  if (pSequence->iSize-iIndex > 0)
    memmove(&pSequence->ppItems[iIndex], &pSequence->ppItems[iIndex+1],
	    pSequence->iSize-iIndex-1);
  pSequence->iSize--;
  if (pSequence->iSize == 0) {
    FREE(pSequence->ppItems);
    pSequence->ppItems= NULL;
  } else
    pSequence->ppItems= (void *) REALLOC(pSequence->ppItems,
					 pSequence->iSize*sizeof(void *));
  return 0;
}

// ----- sequence_for_each ------------------------------------------
/**
 *
 */
void sequence_for_each(SSequence * pSequence, FSeqForEach fForEach,
		       void * pContext)
{
  int iIndex;

  for (iIndex= 0; iIndex < pSequence->iSize; iIndex++)
    fForEach(pSequence->ppItems[iIndex], pContext);
}

// ----- sequence_copy ----------------------------------------------
/**
 *
 */
SSequence * sequence_copy(SSequence * pSequence, FSeqCopyItem fCopyItem)
{
  int iIndex;

  SSequence * pNewSequence= sequence_create(pSequence->fCompare,
					    pSequence->fDestroy);
  sequence_copy_count++;
  pNewSequence->iSize= pSequence->iSize;
  if (pNewSequence->iSize > 0) {
    pNewSequence->ppItems= MALLOC(sizeof(void *)*pNewSequence->iSize);
    if (fCopyItem == NULL)
      memcpy(pNewSequence->ppItems, pSequence->ppItems,
	     sizeof(void *)*pNewSequence->iSize);
    else
      for (iIndex= 0; iIndex < pNewSequence->iSize; iIndex++)
	pNewSequence->ppItems[iIndex]= fCopyItem(pSequence->ppItems[iIndex]);
  }
  return pNewSequence;
}
