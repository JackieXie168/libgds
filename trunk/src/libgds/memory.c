// ==================================================================
// @(#)memory.c
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 29/11/2002
// @lastdate 20/08/2003
// ==================================================================

#include <stdio.h>
#include <stdlib.h>

unsigned long int uAllocCount= 0;

// ----- memalloc ---------------------------------------------------
/**
 *
 */
void * memalloc(size_t size)
{
  void * pNewPtr= malloc(size);

  if (pNewPtr == NULL) {
    perror("memalloc: ");
    exit(EXIT_FAILURE);
  }

  uAllocCount++;

  return pNewPtr;
}

// ----- memrealloc -------------------------------------------------
/**
 *
 */
extern void * memrealloc(void * pPtr, size_t size)
{
  void * pNewPtr= realloc(pPtr, size);

  if (pNewPtr == NULL) {
    perror("memrealloc: ");
    exit(EXIT_FAILURE);
  }
    
  return pNewPtr;
}

// ----- memfree ----------------------------------------------------
/**
 *
 */
void memfree(void * pPtr)
{
  if (uAllocCount > 0)
    uAllocCount--;
  else {
    fprintf(stderr, "memfree: alloc-count == 0");
    exit(EXIT_FAILURE);
  }

  free(pPtr);
}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void _memory_destroy() __attribute__((destructor));

// ----- _memory_destroy --------------------------------------------
/**
 *
 */
void _memory_destroy()
{
  if (uAllocCount > 0)
    fprintf(stderr, "WARNING: memory leak (%lu) !\n", uAllocCount);
}
