// ==================================================================
// @(#)lifo.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Damien Saucez (damien.saucez@uclouvain.be)
// @date 28/11/2002
// $Id: lifo.h 306 2009-03-27 11:56:16Z bquoitin $
// ==================================================================

/**
 * \file
 * Provide a data structure and functions to manage lifo queues.
 */

#ifndef __GDS_LIFO_H__
#define __GDS_LIFO_H__

#include <libgds/types.h>

#define LIFO_OPTION_GROW_LINEAR      0x01
#define LIFO_OPTION_GROW_EXPONENTIAL 0x02

/** Destroy a lifo item. */
typedef void (*gds_lifo_destroy_f)(void ** item_ref);

typedef struct gds_lifo_t {
  unsigned int          max_depth;
  uint8_t               options;
  unsigned int          current_depth;
  gds_lifo_destroy_f    destroy;
  void               ** items;
} gds_lifo_t;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ lifo_create ]--------------------------------------------
  /**
   * Create a lifo queue.
   *
   * \param max_depth is the maximum depth.
   * \param destroy   is the destroy callback function (can be NULL).
   */
  gds_lifo_t * lifo_create(unsigned int max_depth,
			   gds_lifo_destroy_f destroy);

  // -----[ lifo_destroy ]-------------------------------------------
  /**
   * Destroy a lifo queue.
   *
   * If the destroy callback is not NULL, it will be called for each
   * item in the queue.
   *
   * \param lifo_ref is the pointer to the lifo to be destroyed.
   */
  void lifo_destroy(gds_lifo_t ** lifo_ref);

  // -----[ lifo_set_option ]----------------------------------------
  void lifo_set_option(gds_lifo_t * lifo, uint8_t flag, int state);

  // -----[ lifo_push ]----------------------------------------------
  /**
   * Push an item onto the lifo queue.
   *
   * \param lifo is the target lifo queue.
   * \param item is the item to be pushed.
   * \retval 0 if the item could be pushed,
   *         or <0 if the lifo is full (i.e. if current depth equals
   *         max depth).
   */
  int lifo_push(gds_lifo_t * lifo, void * item);

  // -----[ lifo_pop ]-----------------------------------------------
  /**
   * Pop an item from the lifo queue.
   *
   * \param lifo is the source lifo queue.
   * \retval the earliest pushed item,
   *         or NULL if the lifo queue is empty.
   */
  void * lifo_pop(gds_lifo_t * lifo);

  // -----[ lifo_depth ]---------------------------------------------
  /**
   * Return the depth of the lifo queue.
   *
   * \param lifo is the target lifo queue.
   * \retval the current depth.
   */
  unsigned int lifo_depth(gds_lifo_t * lifo);

  // -----[ lifo_get_at ]--------------------------------------------
  /**
   * Return the item at a given position in the lifo queue.
   *
   * \param lifo is the target lifo queue.
   * \param index is the location of the item searched.
   * \retval the searched item.
   */
  void * lifo_get_at(gds_lifo_t * lifo, unsigned int index);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_LIFO_H__ */
