// ==================================================================
// @(#)memory.c
//
// @author  Bruno Quoitin (bqu@info.ucl.ac.be), 
//	    Sebastien Tandel (standel@info.ucl.ac.be)
// @date 29/11/2002
// @lastdate 08/02/2005
// ==================================================================

#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <libgds/memory.h>

/* Implementation of a kind of garbage collector to detect the origin 
 * of some memory bugs. It can be used to debug two types of memory bugs :
 * 1) memory leak : At the end of the process, it displays all the memory
 * allocated by this library and not freed by it. Furthermore, the file name
 * and the line where the allocation has been made is displayed too.
 * 2) freeing a memory space allocated by an other way than the library : When
 * this kind of memory space is freed, the gc will notice that this part of
 * memory hasn't been allocated by this library. Furthermore, the file name and
 * the line where the free has been made is displayed too.
 * 
 * As it is a memory and process consumer, to be compiled, it has to be
 * configured first with the option '--enable-memory-debug'.
 * 
 */
#ifdef HAVE_MEMORY_DEBUG
#include <string.h>
#include <libgds/list.h>

typedef struct {
  unsigned int uPtrAddr;
  unsigned int uSize;
  char * pcFileName;
  int iLineNumber;
}SMemAlloc;
#endif

// ----- uAllocCount -----
/* uAllocCount tracks the balance of calls to memalloc/memfree. The
 * allocation count is initialized to -1 and re-initialized to 0 by
 * the global ctor function _memory_init. If memalloc detects that the
 * allocation counter is -1, this means that the ctor function has not
 * yet been called (problem during linking process ?) */
static long int dAllocCount= -1;

// ----- uFlags -----
static uint8_t uFlags= 0;

#ifdef HAVE_MEMORY_DEBUG
static SList * plMemAlloc= NULL;
static long int iAllocatedBytes= 0;
static long int iFreedBytes= 0;
static long int iMaxAllocatedAtOneTime= 0;

SMemAlloc * memory_alloc_init(void * uPtrAddr, unsigned int uSize, 
			char * pcFileName, int iLineNumber)
{
  SMemAlloc * pAlloc = malloc(sizeof(SMemAlloc));

  pAlloc->uPtrAddr = (unsigned int)uPtrAddr;
  pAlloc->uSize = uSize;
  pAlloc->pcFileName = strdup(pcFileName);
  pAlloc->iLineNumber = iLineNumber;

  return pAlloc;
}

int memory_alloc_compare(void * pItem1, void * pItem2)
{
  SMemAlloc * pAlloc1 = (SMemAlloc *)pItem1;
  SMemAlloc * pAlloc2 = (SMemAlloc *)pItem2;

  if (pAlloc1->uPtrAddr < pAlloc2->uPtrAddr)
    return -1;
  else if (pAlloc1->uPtrAddr > pAlloc2->uPtrAddr)
    return 1;
  else
    return 0;
}

void memory_alloc_destroy(void ** pItem)
{
  SMemAlloc ** pAlloc = (SMemAlloc **)pItem;
  
  if ((*pAlloc) != NULL) {
    if ((*pAlloc)->pcFileName != NULL)
      free((*pAlloc)->pcFileName);
    free(*pAlloc);
    *pAlloc=NULL;
  }
}

void memory_alloc_for_each(void * pItem, void * pContext)
{
  SMemAlloc * pAlloc = (SMemAlloc *)pItem;
  fprintf(stderr, "[%p] : %u bytes memory leak in %s (line %d) ... ", 
      (void *)pAlloc->uPtrAddr,
      pAlloc->uSize, 
      pAlloc->pcFileName, 
      pAlloc->iLineNumber);
  //free the memory allocated by the process.
  free ((void *)pAlloc->uPtrAddr);
  fprintf(stderr, "freed\n");
}

void memory_alloc_debug()
{
  plMemAlloc = list_create(memory_alloc_compare, memory_alloc_destroy, 15000);
}
#endif

// ----- memalloc ---------------------------------------------------
/**
 *
 */
void * memalloc(size_t size, char * pcFileName, int iLineNumber)
{
  void * pNewPtr = malloc(size);
#ifdef HAVE_MEMORY_DEBUG
  SMemAlloc * pAlloc = NULL;
#endif

  //printf("memalloc:debug %ld\n", dAllocCount);

  if (dAllocCount < 0) {
    fprintf(stderr, "memalloc: dtor function _memory_init has not yet ");
    fprintf(stderr, "been called. Check the linking process !!!\n");
    exit(EXIT_FAILURE);
  }
  
  if (pNewPtr == NULL) {
    perror("memalloc: ");
    exit(EXIT_FAILURE);
  }

#ifdef HAVE_MEMORY_DEBUG
  if (plMemAlloc != NULL) {
    pAlloc = memory_alloc_init(pNewPtr, size, 
				pcFileName, iLineNumber);
    if (list_add(plMemAlloc, pAlloc) == -1) {
      fprintf(stderr, "allocation already made\n");
      exit(EXIT_FAILURE);
    }
    iAllocatedBytes += size;
    if (iMaxAllocatedAtOneTime < iAllocatedBytes-iFreedBytes)
      iMaxAllocatedAtOneTime = iAllocatedBytes-iFreedBytes;
  }
#endif
  
  dAllocCount++;
  
  return pNewPtr;
}

// ----- memrealloc -------------------------------------------------
/**
 *
 */
extern void * memrealloc(void * pPtr, size_t size, char * pcFileName, int iLineNumber)
{
  void * pNewPtr= realloc(pPtr, size);
#ifdef HAVE_MEMORY_DEBUG
  SMemAlloc * pAlloc=NULL,  * pAllocSearched= NULL;
  int iIndex= 0;
#endif


  if (pNewPtr == NULL) {
    perror("memrealloc: ");
    exit(EXIT_FAILURE);
  }
    
#ifdef HAVE_MEMORY_DEBUG
  if (plMemAlloc != NULL) {
    if (pNewPtr != pPtr) {
      pAlloc = memory_alloc_init(pPtr, 0, "", 0);
      if (list_find_index(plMemAlloc, pAlloc, &iIndex) == 0) {

	//useful for some stat
	pAllocSearched = list_get_index(plMemAlloc, iIndex);
	iAllocatedBytes -= pAllocSearched->uSize;
	iAllocatedBytes += size;
	if (iMaxAllocatedAtOneTime < iAllocatedBytes-iFreedBytes)
	  iMaxAllocatedAtOneTime = iAllocatedBytes-iFreedBytes;
	
	list_delete(plMemAlloc, iIndex);

	free(pAlloc);
	pAlloc = memory_alloc_init(pNewPtr, size, 
				    pcFileName, iLineNumber);
	if (list_add(plMemAlloc, pAlloc) == -1) {
	  free(pAlloc);
	  exit(EXIT_FAILURE);
	}
      } else {
	free(pAlloc);
	exit(EXIT_FAILURE);
      }
    }
  }
#endif

  return pNewPtr;
}

// ----- memfree ----------------------------------------------------
/**
 *
 */
void memfree(void * pPtr, 
    char * pcFileName, int iLineNumber)
{
#ifdef HAVE_MEMORY_DEBUG
  SMemAlloc * pAlloc = NULL, * pAllocSearched = NULL;
  int iIndex = 0;

  if (plMemAlloc != NULL) {
    pAlloc = memory_alloc_init(pPtr, 0, "", 0);
    if (list_find_index(plMemAlloc, pAlloc, &iIndex) != -1) {
      //some stat
      pAllocSearched = list_get_index(plMemAlloc, iIndex);
      iFreedBytes += pAllocSearched->uSize;

      list_delete(plMemAlloc, iIndex);
    } else 
      fprintf(stderr, "[%p] : memory not allocated by MALLOC : %s (line %d)\n", 
	  pPtr, 
	  pcFileName, 
	  iLineNumber);
    free(pAlloc);
  }
#endif
  if (dAllocCount > 0)
    free(pPtr);
  else {
    fprintf(stderr, "memfree: alloc-count == %ld : %s (line %d)\n",
	    dAllocCount, pcFileName, iLineNumber);
    //exit(EXIT_FAILURE);
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

void _memory_init() __attribute__((constructor));
void _memory_destroy() __attribute__((destructor));

// ----- _memory_init -----------------------------------------------
/**
 *
 */
void _memory_init()
{
  dAllocCount= 0;
#ifdef HAVE_MEMORY_DEBUG
  memory_alloc_debug();
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

#ifdef HAVE_MEMORY_DEBUG
  if (plMemAlloc != NULL) {
    list_for_each(plMemAlloc, memory_alloc_for_each, NULL);
    list_destroy(&plMemAlloc);
    fprintf(stderr, "total allocated bytes : %li\n", iAllocatedBytes);
    fprintf(stderr, "total freed bytes     : %li\n", iFreedBytes);
    fprintf(stderr, "max allocated         : %li\n", iMaxAllocatedAtOneTime);
  }
#endif
}
