// ==================================================================
// @(#)memory_debug.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 17/05/2005
// @lastdate 20/12/2007
// ==================================================================
// Implementation of a kind of garbage collector to detect the origin
// of some memory bugs. It can be used to debug two types of memory
// bugs:
//   1) memory leaks: at the end of the process, it displays all the
//      memory allocated by this library and not freed by
//      it. Furthermore, the file name and the line where the
//      allocation has been made is displayed too.
//   2) untracked allocations (freeing a memory space allocated by an
//      other way than the library): when this kind of memory space is
//      freed, the gc will notice that this part of memory hasn't been
//      allocated by this library. Furthermore, the file name and the
//      line where the free has been made is displayed too.
//
// As it is memory and CPU hungry, this part must be explicitly
// compiled into libgds by configuring the option
// '--enable-memory-debug'
// ==================================================================

#include <libgds/memory_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgds/list.h>

typedef struct {
  void * pPtrAddr;
  unsigned int uSize;
  char * pcFileName;
  int iLineNumber;
} SMemAlloc;

static SList * plMemAlloc= NULL;
static long int iAllocatedBytes= 0;
static long int iFreedBytes= 0;
static long int iMaxAllocatedAtOneTime= 0;

// -----[ _mem_dbg_alloc_init ]--------------------------------------
static SMemAlloc * _mem_dbg_alloc_init(void * pPtrAddr, unsigned int uSize, 
				       char * pcFileName, int iLineNumber)
{
  SMemAlloc * pAlloc= malloc(sizeof(SMemAlloc));

  pAlloc->pPtrAddr= pPtrAddr;
  pAlloc->uSize= uSize;
  pAlloc->pcFileName= strdup(pcFileName);
  pAlloc->iLineNumber= iLineNumber;

  return pAlloc;
}

// -----[ _mem_dbg_alloc_cmp ]---------------------------------------
static int _mem_dbg_alloc_cmp(void * pItem1, void * pItem2)
{
  SMemAlloc * pAlloc1 = (SMemAlloc *)pItem1;
  SMemAlloc * pAlloc2 = (SMemAlloc *)pItem2;

  if (pAlloc1->pPtrAddr < pAlloc2->pPtrAddr)
    return -1;
  else if (pAlloc1->pPtrAddr > pAlloc2->pPtrAddr)
    return 1;
  else
    return 0;
}

// -----[ _mem_dbg_alloc_destroy ]-----------------------------------
static void _mem_dbg_alloc_destroy(void ** pItem)
{
  SMemAlloc ** pAlloc = (SMemAlloc **)pItem;
  
  if ((*pAlloc) != NULL) {
    if ((*pAlloc)->pcFileName != NULL)
      free((*pAlloc)->pcFileName);
    free(*pAlloc);
    *pAlloc=NULL;
  }
}

// -----[ _mem_dbg_alloc_for_each ]----------------------------------
static void _mem_dbg_alloc_for_each(void * pItem, void * pContext)
{
  SMemAlloc * pAlloc = (SMemAlloc *)pItem;
  fprintf(stderr, "[%p] : %u bytes memory leak in %s (line %d) ... ", 
	  pAlloc->pPtrAddr,
	  pAlloc->uSize, 
	  pAlloc->pcFileName, 
	  pAlloc->iLineNumber);
  //free the memory allocated by the process.
  // No, we shouldn't do that !
  /*free (pAlloc->pPtrAddr);
    fprintf(stderr, "freed");*/
  fprintf(stderr, "\n");
}

// -----[ memory_debug_track_alloc ]---------------------------------
void memory_debug_track_alloc(void * pNewPtr,
			      size_t size,
			      char * pcFileName,
			      int iLineNumber)
{
  SMemAlloc * pAlloc = NULL;

  if (plMemAlloc != NULL) {
    pAlloc= _mem_dbg_alloc_init(pNewPtr, size, 
				pcFileName, iLineNumber);
    if (list_add(plMemAlloc, pAlloc) == -1) {
      fprintf(stderr, "allocation already made\n");
      exit(EXIT_FAILURE);
    }
    iAllocatedBytes+= size;
    if (iMaxAllocatedAtOneTime < iAllocatedBytes-iFreedBytes)
      iMaxAllocatedAtOneTime= iAllocatedBytes-iFreedBytes;
  }
}

// -----[ memory_debug_track_realloc ]-------------------------------
void memory_debug_track_realloc(void * pNewPtr,
				void * pPtr,
				size_t size,
				char * pcFileName,
				int iLineNumber)
{
  SMemAlloc * pAlloc = NULL, * pAllocSearched = NULL;
  int iIndex = 0;

  if (plMemAlloc != NULL) {
    if (pNewPtr != pPtr) {
      pAlloc= _mem_dbg_alloc_init(pPtr, 0, "", 0);
      if (list_find_index(plMemAlloc, pAlloc, &iIndex) == 0) {

	//useful for some stat
	pAllocSearched= list_get_index(plMemAlloc, iIndex);
	iAllocatedBytes-= pAllocSearched->uSize;
	iAllocatedBytes+= size;
	if (iMaxAllocatedAtOneTime < iAllocatedBytes-iFreedBytes)
	  iMaxAllocatedAtOneTime= iAllocatedBytes-iFreedBytes;
	
	list_delete(plMemAlloc, iIndex);

	free(pAlloc);
	pAlloc= _mem_dbg_alloc_init(pNewPtr, size, 
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
}

// -----[ memory_debug_track_free ]----------------------------------
void memory_debug_track_free(void * pPtr,
			     char * pcFileName,
			     int iLineNumber)
{
  SMemAlloc * pAlloc = NULL, * pAllocSearched = NULL;
  int iIndex = 0;

  if (plMemAlloc != NULL) {
    pAlloc= _mem_dbg_alloc_init(pPtr, 0, "", 0);
    if (list_find_index(plMemAlloc, pAlloc, &iIndex) != -1) {
      //some stat
      pAllocSearched= list_get_index(plMemAlloc, iIndex);
      iFreedBytes+= pAllocSearched->uSize;
      list_delete(plMemAlloc, iIndex);
    } else 
      fprintf(stderr, "[%p] : memory not allocated by MALLOC : %s (line %d)\n",
	      pPtr, pcFileName, iLineNumber);
    free(pAlloc);
  }
}

// -----[ memory_debug_init ]----------------------------------------
void memory_debug_init(int iTrack)
{
  if (iTrack) {
    plMemAlloc= list_create(_mem_dbg_alloc_cmp,
			    _mem_dbg_alloc_destroy, 15000);
  }
}

// -----[ memory_debug_destroy ]-------------------------------------
void memory_debug_destroy()
{
  if (plMemAlloc != NULL) {
    list_for_each(plMemAlloc, _mem_dbg_alloc_for_each, NULL);
    list_destroy(&plMemAlloc);
    fprintf(stderr, "total allocated bytes : %li\n", iAllocatedBytes);
    fprintf(stderr, "total freed bytes     : %li\n", iFreedBytes);
    fprintf(stderr, "max allocated         : %li\n", iMaxAllocatedAtOneTime);
  }
}
