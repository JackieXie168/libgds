// ==================================================================
// @(#)fifo.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 28/11/2002
// @lastdate 28/11/2002
// ==================================================================

#ifndef __FIFO_H__
#define __FIFO_H__

#include <libgds/types.h>

// ----- FFIFODestroy -----------------------------------------------
typedef void (*FFIFODestroy)(void ** ppItem);

typedef struct {
  uint32_t uMaxDepth;
  uint32_t uStartIndex;
  uint32_t uCurrentDepth;
  FFIFODestroy fDestroy;
  void ** ppItems;
} SFIFO;

// ----- fifo_create ------------------------------------------------
extern SFIFO * fifo_create(uint32_t uMaxDepth, FFIFODestroy fDestroy);
// ----- fifo_destroy -----------------------------------------------
extern void fifo_destroy(SFIFO ** ppFIFO);
// ----- fifo_push --------------------------------------------------
extern int fifo_push(SFIFO * pFIFO, void * pItem);
// ----- fifo_pop ---------------------------------------------------
extern void * fifo_pop(SFIFO * pFIFO);

#endif
