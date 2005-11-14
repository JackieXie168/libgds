// ==================================================================
// @(#)memory.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 29/11/2002
// @lastdate 30/11/2002
// ==================================================================

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdlib.h>

extern unsigned long uAllocCount;

// ----- memalloc ---------------------------------------------------
extern void * memalloc(size_t size);
// ----- memrealloc -------------------------------------------------
extern void * memrealloc(void * pPtr, size_t size);
// ----- memfree ----------------------------------------------------
extern void memfree(void * pPtr);

#define MALLOC(s) memalloc(s)
#define REALLOC(p, s) memrealloc(p, s)
#define FREE(p) memfree(p)

#endif
