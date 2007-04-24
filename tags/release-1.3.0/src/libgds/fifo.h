// ==================================================================
// @(#)fifo.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 28/11/2002
// @lastdate 17/03/2007
// ==================================================================

#ifndef __FIFO_H__
#define __FIFO_H__

#include <libgds/types.h>

#define FIFO_OPTION_GROW_LINEAR      0x01
#define FIFO_OPTION_GROW_EXPONENTIAL 0x02

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

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ fifo_create ]--------------------------------------------
  SFIFO * fifo_create(uint32_t uMaxDepth, FFIFODestroy fDestroy);
  // -----[ fifo_destroy ]-------------------------------------------
  void fifo_destroy(SFIFO ** ppFIFO);
  // -----[ fifo_set_option ]----------------------------------------
  void fifo_set_option(SFIFO * pFIFO, uint8_t uFlag, int iState);
  // -----[ fifo_push ]----------------------------------------------
  int fifo_push(SFIFO * pFIFO, void * pItem);
  // -----[ fifo_pop ]-----------------------------------------------
  void * fifo_pop(SFIFO * pFIFO);
  // -----[ fifo_depth ]---------------------------------------------
  uint32_t fifo_depth(SFIFO * pFIFO);

#ifdef __cplusplus
}
#endif

#endif
