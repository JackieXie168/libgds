// ==================================================================
// @(#)memory.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 29/11/2002
// @lastdate 26/05/2004
// ==================================================================

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdlib.h>

#include <libgds/types.h>

#define MEM_FLAG_WARN_LEAK 0x01 /* Display a warning in case of memory
				   leak when the memory.o object is
				   destroyed (note that memory.o must
				   be the last object in the .DTOR
				   list) */

// ----- memalloc ---------------------------------------------------
extern void * memalloc(size_t size, char * pcFileName, 
			int iLineNumber);
// ----- memrealloc -------------------------------------------------
extern void * memrealloc(void * pPtr, size_t size, char * pcFileName,
			int iLineNumber);
// ----- memfree ----------------------------------------------------
extern void memfree(void * pPtr, char * pcFileName, 
		    int iLineNumber);
// ----- mem_alloc_cnt ----------------------------------------------
extern long int mem_alloc_cnt();
// ----- mem_flag_set -----------------------------------------------
extern void mem_flag_set(uint8_t uFlag, int iState);
// ----- mem_flag_get -----------------------------------------------
extern int mem_flag_get(uint8_t uFlag);

#define MALLOC(s) memalloc(s, __FILE__, __LINE__)
#define REALLOC(p, s) memrealloc(p, s, __FILE__, __LINE__)
#define FREE(p) memfree(p, __FILE__, __LINE__)

#endif
