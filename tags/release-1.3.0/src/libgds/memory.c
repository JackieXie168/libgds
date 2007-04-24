// ==================================================================
// @(#)memory.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be), 
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 17/05/2005
// @lastdate 16/01/2007
// ==================================================================

#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <libgds/memory.h>

#include <libgds/memory_debug.h>

// ----- uAllocCount -----
/* uAllocCount tracks the balance of calls to memalloc/memfree. The
 * allocation count is initialized to -1 and re-initialized to 0 by
 * the global ctor function _memory_init. If memalloc detects that the
 * allocation counter is -1, this means that the ctor function has not
 * yet been called (problem during linking process ?) */
static long int dAllocCount= -1;

// ----- uFlags -----
static uint8_t uFlags= 0;

// -----[ memalloc ]-------------------------------------------------
/**
 *
 */
void * memalloc(size_t size, char * pcFileName, int iLineNumber)
{
  void * pNewPtr= malloc(size);

  if (dAllocCount < 0) {
    fprintf(stderr, "memalloc: dtor function _memory_init has not yet ");
    fprintf(stderr, "been called. Check the linking process !!!\n");
    exit(EXIT_FAILURE);
  }
  
  if (pNewPtr == NULL) {
    perror("memalloc: ");
    exit(EXIT_FAILURE);
  }

#ifdef MEMORY_DEBUG
  memory_debug_track_alloc(pNewPtr, size, pcFileName, iLineNumber);
#endif
  
  dAllocCount++;
  
  return pNewPtr;
}

// -----[ memrealloc ]-----------------------------------------------
/**
 *
 */
extern void * memrealloc(void * pPtr, size_t size,
			 char * pcFileName, int iLineNumber)
{
  void * pNewPtr= realloc(pPtr, size);

  if (pNewPtr == NULL) {
    perror("memrealloc: ");
    exit(EXIT_FAILURE);
  }
    
#ifdef MEMORY_DEBUG
  memory_debug_track_realloc(pNewPtr, pPtr, size, pcFileName, iLineNumber);
#endif

  return pNewPtr;
}

// -----[ memfree ]--------------------------------------------------
/**
 *
 */
void memfree(void * pPtr, char * pcFileName, int iLineNumber)
{
#ifdef MEMORY_DEBUG
  memory_debug_track_free(pPtr, pcFileName, iLineNumber);
#endif

  if (dAllocCount > 0) {
    free(pPtr);
  } else {
#ifdef MEMORY_DEBUG
    fprintf(stderr, "memfree: alloc-count == %ld : %s (line %d)\n",
	    dAllocCount, pcFileName, iLineNumber);
#endif
  }
  dAllocCount--;

}

// ----- mem_alloc_cnt ----------------------------------------------
/**
 *
 */
long int mem_alloc_cnt()
{
  return dAllocCount;
}

// ----- mem_flag_set -----------------------------------------------
/**
 *
 */
void mem_flag_set(uint8_t uFlag, int iState)
{
  if (iState)
    uFlags|= uFlag;
  else
    uFlags&= ~uFlag;
}

// ----- mem_flag_get -----------------------------------------------
/**
 *
 */
int mem_flag_get(uint8_t uFlag)
{
  return (uFlags & uFlag);
}

/////////////////////////////////////////////////////////////////////
// INITIALIZATION AND FINALIZATION FUNCTIONS
/////////////////////////////////////////////////////////////////////

// ----- _memory_init -----------------------------------------------
/**
 *
 */
void _memory_init()
{
  dAllocCount= 0;

#ifdef MEMORY_DEBUG
  memory_debug_init(mem_flag_get(MEM_FLAG_TRACK_LEAK));
#endif
}

// ----- _memory_destroy --------------------------------------------
/**
 *
 */
void _memory_destroy()
{
  if (mem_flag_get(MEM_FLAG_WARN_LEAK) && (dAllocCount > 0))
    fprintf(stderr, "WARNING: memory leak (%lu) !\n", dAllocCount);

#ifdef MEMORY_DEBUG
  memory_debug_destroy();
#endif
}
