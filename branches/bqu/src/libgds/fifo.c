// ==================================================================
// @(#)fifo.c
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 28/11/2002
// @lastdate 30/11/2002
// ==================================================================

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

// ----- fifo_push --------------------------------------------------
/**
 *
 */
int fifo_push(SFIFO * pFIFO, void * pItem)
{
  if (pFIFO->uCurrentDepth < pFIFO->uMaxDepth) {
    pFIFO->ppItems[(pFIFO->uStartIndex+
		    pFIFO->uCurrentDepth) % pFIFO->uMaxDepth]= pItem;
    pFIFO->uCurrentDepth++;
    return 0;
  } else
    return -1;
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
