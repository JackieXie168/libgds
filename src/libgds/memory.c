// ==================================================================
// @(#)memory.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 29/11/2002
// @lastdate 04/03/2004
// ==================================================================

#include <stdio.h>
#include <stdlib.h>

// ----- uAllocCount -----
// uAllocCount tracks the balance of calls to memalloc/memfree. The
// allocation count is initialized to -1 and re-initialized to 0 by
// the global ctor function _memory_init. If memalloc detects that the
// allocation counter is -1, this means that the ctor function has not
// yet been called (problem during linking process ?)
static unsigned long int uAllocCount= -1;

// ----- memalloc ---------------------------------------------------
/**
 *
 */
void * memalloc(size_t size)
{
  void * pNewPtr= malloc(size);

  if (uAllocCount < 0) {
    fprintf(stderr, "memalloc: dtor function _memory_init has not yet ");
    fprintf(stderr, "been called. Check the linking process !!!\n");
    exit(EXIT_FAILURE);
  }	
  
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
// INITIALIZATION AND FINALIZATION FUNCTIONS
/////////////////////////////////////////////////////////////////////

void _memory_init() __attribute__((constructor));
void _memory_destroy() __attribute__((destructor));

// ----- _memory_init -----------------------------------------------
/**
 *
 */
void _memory_init()
{
  uAllocCount= 0;
}

// ----- _memory_destroy --------------------------------------------
/**
 *
 */
void _memory_destroy()
{
  if (uAllocCount > 0)
    fprintf(stderr, "WARNING: memory leak (%lu) !\n", uAllocCount);
}
