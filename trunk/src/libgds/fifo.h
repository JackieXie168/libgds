// ==================================================================
// @(#)fifo.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Damien Saucez (damien.saucez@uclouvain.be)
// @date 28/11/2002
// $Id$
// ==================================================================

#ifndef __GDS_FIFO_H__
#define __GDS_FIFO_H__

#include <libgds/types.h>

#define FIFO_OPTION_GROW_LINEAR      0x01
#define FIFO_OPTION_GROW_EXPONENTIAL 0x02

// ----- FFIFODestroy -----------------------------------------------
typedef void (*FFIFODestroy)(void ** ppItem);

typedef struct fifo_t {
  uint32_t        max_depth;
  uint8_t         options;
  uint32_t        start_index;
  uint32_t        current_depth;
  FFIFODestroy    fDestroy;
  void         ** items;
} gds_fifo_t;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ fifo_create ]--------------------------------------------
  gds_fifo_t * fifo_create(uint32_t max_depth, FFIFODestroy destroy);
  // -----[ fifo_destroy ]-------------------------------------------
  void fifo_destroy(gds_fifo_t ** fifo_ref);
  // -----[ fifo_set_option ]----------------------------------------
  void fifo_set_option(gds_fifo_t * fifo, uint8_t flag, int state);
  // -----[ fifo_push ]----------------------------------------------
  int fifo_push(gds_fifo_t * fifo, void * item);
  // -----[ fifo_pop ]-----------------------------------------------
  void * fifo_pop(gds_fifo_t * fifo);
  // -----[ fifo_depth ]---------------------------------------------
  uint32_t fifo_depth(gds_fifo_t * fifo);
  // -----[ fifo_get_at ]--------------------------------------------
  void * fifo_get_at(gds_fifo_t * fifo, unsigned int index);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_FIFO_H__ */
