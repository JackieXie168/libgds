// ==================================================================
// @(#)fifo.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 28/11/2002
// @lastdate 08/03/2004
// ==================================================================

#include <stdio.h>
#include <stdlib.h>

#include <libgds/fifo.h>
#include <libgds/memory.h>

// ----- fifo_create ------------------------------------------------
/**
 *
 */
SFIFO * fifo_create(uint32_t uMaxDepth, FFIFODestroy fDestroy)
{
  SFIFO * pFIFO= (SFIFO *) MALLOC(sizeof(SFIFO));
  pFIFO->uMaxDepth= uMaxDepth;
  pFIFO->uStartIndex= 0;
  pFIFO->uCurrentDepth= 0;
  pFIFO->fDestroy= fDestroy;
  pFIFO->ppItems= (void **) MALLOC(sizeof(void *)*uMaxDepth);
  return pFIFO;
}

// ----- fifo_destroy -----------------------------------------------
/**
 *
 */
void fifo_destroy(SFIFO ** ppFIFO)
{
  int iIndex;

  if (*ppFIFO != NULL) {
    if ((*ppFIFO)->fDestroy != NULL)
      for (iIndex= 0; iIndex < (*ppFIFO)->uCurrentDepth; iIndex++)
	(*ppFIFO)->fDestroy(&(*ppFIFO)->ppItems[((*ppFIFO)->uStartIndex+
						 iIndex) %
					       (*ppFIFO)->uMaxDepth]);
    FREE((*ppFIFO)->ppItems);
    (*ppFIFO)->ppItems= NULL;
    FREE(*ppFIFO);
    *ppFIFO= NULL;
  }
}

// ----- fifo_set_option ------------------------------------------
/**
 *
 */
void fifo_set_option(SFIFO * pFIFO, uint8_t uOption, int iState)
{
  if (iState)
    pFIFO->uOptions|= uOption;
  else
    pFIFO->uOptions&= ~uOption;
}

// ----- fifo_grow ------------------------------------------------
/**
 *
 */
int fifo_grow(SFIFO * pFIFO)
{
  uint32_t uNewDepth= 0;

  if (pFIFO->uOptions & FIFO_OPTION_GROW_EXPONENTIAL)
    uNewDepth= pFIFO->uMaxDepth * 2;

  if (uNewDepth > pFIFO->uMaxDepth) {

    // Re-allocete FIFO space
    pFIFO->ppItems= REALLOC(pFIFO->ppItems,
			    (sizeof(void *)*uNewDepth));

    // Move exiting items
    // (start+depth % max_depth) items must be moved in newly allocated space
    memcpy(&pFIFO->ppItems[pFIFO->uMaxDepth], pFIFO->ppItems,
	   (pFIFO->uStartIndex + pFIFO->uCurrentDepth) % pFIFO->uMaxDepth);
    pFIFO->uMaxDepth= uNewDepth;
    
  }
  return 0;
}

// ----- fifo_push --------------------------------------------------
/**
 *
 */
int fifo_push(SFIFO * pFIFO, void * pItem)
{

  // If there is not enough space in the queue, try to grow it
  if (pFIFO->uCurrentDepth >= pFIFO->uMaxDepth)
    if (fifo_grow(pFIFO))
      return -1;

  pFIFO->ppItems[(pFIFO->uStartIndex+
		  pFIFO->uCurrentDepth) % pFIFO->uMaxDepth]= pItem;
  pFIFO->uCurrentDepth++;
  return 0;
}

// ----- fifo_pop ---------------------------------------------------
/**
 *
 */
void * fifo_pop(SFIFO * pFIFO)
{
  void * pItem= NULL;

  if (pFIFO->uCurrentDepth > 0) {
    pItem= pFIFO->ppItems[pFIFO->uStartIndex];
    pFIFO->uStartIndex= (pFIFO->uStartIndex+1) % pFIFO->uMaxDepth;
    pFIFO->uCurrentDepth--;
  }
  return pItem;
}
