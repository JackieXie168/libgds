// ==================================================================
// @(#)fifo.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Damien Saucez (damien.saucez@uclouvain.be)
// @date 28/11/2002
// $Id: fifo.h 306 2009-03-27 11:56:16Z bquoitin $
// ==================================================================

/**
 * \file
 * Provide a data structure and functions to manage FIFO queues.
 */

#ifndef __GDS_FIFO_TUNABLE_H__
#define __GDS_FIFO_TUNABLE_H__

#include <libgds/types.h>

#define FIFO_OPTION_GROW_LINEAR      0x01
#define FIFO_OPTION_GROW_EXPONENTIAL 0x02

/** Destroy a FIFO item. */
typedef void (*gds_fifo_destroy_f)(void ** item_ref);

typedef struct gds_fifo_tunable_t {
  unsigned int          max_depth;
  uint8_t               options;
  unsigned int          start_index;
  unsigned int          current_depth;
  gds_fifo_destroy_f    destroy;
  void               ** items;
} gds_fifo_tunable_t;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ fifo_create ]--------------------------------------------
  /**
   * Create a FIFO queue.
   *
   * \param max_depth is the maximum depth.
   * \param destroy   is the destroy callback function (can be NULL).
   */
  gds_fifo_tunable_t * fifo_tunable_create(unsigned int max_depth,
			   gds_fifo_destroy_f destroy);

  // -----[ fifo_destroy ]-------------------------------------------
  /**
   * Destroy a FIFO queue.
   *
   * If the destroy callback is not NULL, it will be called for each
   * item in the queue.
   *
   * \param fifo_ref is the pointer to the FIFO to be destroyed.
   */
  void fifo_tunable_destroy(gds_fifo_tunable_t ** fifo_ref);

  // -----[ fifo_set_option ]----------------------------------------
  void fifo_tunable_set_option(gds_fifo_tunable_t * fifo, uint8_t flag, int state);

  // -----[ fifo_push ]----------------------------------------------
  /**
   * Push an item onto the FIFO queue.
   *
   * \param fifo is the target FIFO queue.
   * \param item is the item to be pushed.
   * \retval 0 if the item could be pushed,
   *         or <0 if the FIFO is full (i.e. if current depth equals
   *         max depth).
   */
  int fifo_tunable_push(gds_fifo_tunable_t * fifo, void * item);


  /*
   set nb as the first elem in the fifo, by swapping with the current first element

   */
  int fifo_tunable_set_first(gds_fifo_tunable_t * fifo, int nb);

    /*
   swap elem in positions nb1 and nb2 in the fifo

   */
  int fifo_tunable_swap(gds_fifo_tunable_t * fifo, int nb1, int nb2);

  /* décaler */
  int fifo_tunable_bringForward(gds_fifo_tunable_t * fifo, int num);


  // -----[ fifo_pop ]-----------------------------------------------
  /**
   * Pop an item from the FIFO queue.
   *
   * \param fifo is the source FIFO queue.
   * \retval the earliest pushed item,
   *         or NULL if the FIFO queue is empty.
   */
  void * fifo_tunable_pop(gds_fifo_tunable_t * fifo);

  // -----[ fifo_depth ]---------------------------------------------
  /**
   * Return the depth of the FIFO queue.
   *
   * \param fifo is the target FIFO queue.
   * \retval the current depth.
   */
  unsigned int fifo_tunable_depth(gds_fifo_tunable_t * fifo);

  // -----[ fifo_get_at ]--------------------------------------------
  /**
   * Return the item at a given position in the FIFO queue.
   *
   * \param fifo is the target FIFO queue.
   * \param index is the location of the item searched.
   * \retval the searched item.
   */
  void * fifo_tunable_get_at(gds_fifo_tunable_t * fifo, unsigned int index);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_FIFO_H__ */
