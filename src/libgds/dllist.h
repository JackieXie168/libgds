// ==================================================================
// @(#)dllist.h
//
// Doubly-linked lists
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 26/07/2006
// $Id$
// ==================================================================

/**
 * \file
 * Provide data structures and functions to manage doubly-linked
 * lists.
 */

#ifndef __GDS_DLLIST_H__
#define __GDS_DLLIST_H__

// -----[ gds_dllist_item_t ]----------------------------------------
typedef struct gds_dllist_item_t {
  struct gds_dllist_item_t * prev;
  struct gds_dllist_item_t * next;
  void * user_data;
} gds_dllist_item_t;

// -----[ gds_dllist_foreach_f ]-------------------------------------
/** Callback function used to traverse a list. */
typedef int  (*gds_dllist_foreach_f)(void * data, void * ctx);

// -----[ gds_dllist_destroy_f ]-------------------------------------
/** Callback function used to destroy an item in a list. */
typedef void (*gds_dllist_destroy_f)(void * user_data);

// -----[ gds_dllist_t ]---------------------------------------------
/**
 * Definition of a doubly-linked list.
 */
typedef struct {
  gds_dllist_item_t    * root;
  gds_dllist_destroy_f   destroy;
} gds_dllist_t;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ dllist_create ]------------------------------------------
  /**
   * Create a doubly-linked list.
   */
  gds_dllist_t * dllist_create(gds_dllist_destroy_f destroy);

  // -----[ dllist_destroy ]-----------------------------------------
  /**
   * Destroy a doubly-linked list.
   */
  void dllist_destroy(gds_dllist_t ** list_ref);

  // -----[ dllist_insert ]------------------------------------------
  int dllist_insert(gds_dllist_t * list, unsigned int index,
		    void * data);

  // -----[ dllist_remove ]------------------------------------------
  int dllist_remove(gds_dllist_t * list, unsigned int index);

  // -----[ dllist_append ]------------------------------------------
  void dllist_append(gds_dllist_t * list, void * data);

  // -----[ dllist_get ]---------------------------------------------
  int dllist_get(gds_dllist_t * list, unsigned int index,
		 void ** data_ref);

  // -----[ dllist_size ]--------------------------------------------
  unsigned int dllist_size(gds_dllist_t * list);

  // -----[ dllist_for_each ]----------------------------------------
  int dllist_for_each(gds_dllist_t * list, void * ctx,
		      gds_dllist_foreach_f foreach);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_DLLIST_H__ */
