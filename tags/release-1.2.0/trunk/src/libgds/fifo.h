// ==================================================================
// @(#)fifo.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 28/11/2002
// @lastdate 08/03/2004
// ==================================================================

#ifndef __FIFO_H__
#define __FIFO_H__

#include <libgds/types.h>

#define FIFO_OPTION_GROW_LINEAR      1
#define FIFO_OPTION_GROW_EXPONENTIAL 2

// ----- FFIFODestroy -----------------------------------------------
typedef void (*FFIFODestroy)(void ** ppItem);

typedef struct {
  uint32_t uMaxDepth;
  uint8_t uOptions;
  uint32_t uStartIndex;
  uint32_t uCurrentDepth;
  FFIFODestroy fDestroy;
  void ** ppItems;
} SFIFO;

// ----- fifo_create ------------------------------------------------
extern SFIFO * fifo_create(uint32_t uMaxDepth, FFIFODestroy fDestroy);
// ----- fifo_destroy -----------------------------------------------
extern void fifo_destroy(SFIFO ** ppFIFO);
// ----- fifo_set_option --------------------------------------------
extern void fifo_set_option(SFIFO * pFIFO, uint8_t uFlag, int iState);
// ----- fifo_push --------------------------------------------------
extern int fifo_push(SFIFO * pFIFO, void * pItem);
// ----- fifo_pop ---------------------------------------------------
extern void * fifo_pop(SFIFO * pFIFO);

#endif
