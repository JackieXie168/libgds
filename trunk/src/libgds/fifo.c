// ==================================================================
// @(#)fifo.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 28/11/2002
// @lastdate 17/03/2007
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <libgds/fifo.h>
#include <libgds/memory.h>

// ----- fifo_create ------------------------------------------------
/**
 *
 */
SFIFO * fifo_create(uint32_t uMaxDepth, FFIFODestroy fDestroy)
{
  SFIFO * pFIFO= (SFIFO *) MALLOC(sizeof(SFIFO));
  pFIFO->uOptions= 0;
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

// ----- _fifo_grow -----------------------------------------------
/**
 *
 */
static int _fifo_grow(SFIFO * pFIFO)
{
  uint32_t uNewDepth= 0;

  // Note: currently, whatever exponential or linear is selected,
  // _fifo_grow will lead to exponential growth
  if (pFIFO->uOptions & FIFO_OPTION_GROW_EXPONENTIAL)
    uNewDepth= pFIFO->uMaxDepth * 2;
  else if (pFIFO->uOptions & FIFO_OPTION_GROW_LINEAR)
    uNewDepth= pFIFO->uMaxDepth * 2;
  else
    return -1;

  if (uNewDepth > pFIFO->uMaxDepth) {

    // Re-allocate FIFO space
    pFIFO->ppItems= REALLOC(pFIFO->ppItems,
			    (sizeof(void *)*uNewDepth));

    // Move exiting items
    if (pFIFO->uCurrentDepth > pFIFO->uMaxDepth-pFIFO->uStartIndex)
      memcpy(&pFIFO->ppItems[pFIFO->uMaxDepth], &pFIFO->ppItems[0],
	     (pFIFO->uCurrentDepth-(pFIFO->uMaxDepth-pFIFO->uStartIndex))*
	     sizeof(void *));
    
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
    if (_fifo_grow(pFIFO) != 0)
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

// -----[ fifo_depth ]---------------------------------------------
/**
 *
 */
uint32_t fifo_depth(SFIFO * pFIFO)
{
  return pFIFO->uCurrentDepth;
}
