// ==================================================================
// @(#)list.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @author Sebastien Tandel (sta@info.ucl.ac.be)
// @date 23/11/2002
// $Id$
// ==================================================================

/**
 * \file
 * Provide data structures and functions to manage a list.
 */

#ifndef __GDS_LIST_H__
#define __GDS_LIST_H__

// -----[ gd_list_cmp_f ]--------------------------------------------
/** 
 * ITEM1 == ITEM2 => 0
 * ITEM1 > ITEM2 => +1
 * ITEM1 < ITEM2 => -1
 */
typedef int (*gds_list_cmp_f)(const void * item1, const void * item2);

// -----[ gds_list_destroy_f ]---------------------------------------
typedef void (*gds_list_destroy_f)(void ** item_ref);

// -----[ gds_list_foreach_f ]---------------------------------------
typedef int (*gds_list_foreach_f)(void * item, void * ctx);

// -----[ gds_list_dup_f ]-------------------------------------------
typedef void * (*gds_list_dup_f)(const void * item);

// -----[ gds_list_ops_t ]-------------------------------------------
/** Virtual methods of a list. */
typedef struct {
  gds_list_cmp_f     cmp;
  gds_list_destroy_f destroy;
} gds_list_ops_t;

// -----[ gds_list_t ]-----------------------------------------------
/** Definition of a list. */
typedef struct {
  /** Number of cells allocated. */
  unsigned int      size;
  /** Number of cells used. */
  unsigned int      length;
  unsigned int      resize_step;
  /** Virtual methods of the list. */
  gds_list_ops_t    ops;
  /** List of items (cells). */
  void           ** items;
} gds_list_t;

#ifdef _cplusplus
extern "C" {
#endif

  // -----[ list_create ]--------------------------------------------
  /**
   * Create a list.
   *
   * \param cmp         is a compare callback function.
   * \param destropy    is an item de-allocation callback function.
   * \param resize_step is the size of the list increase.
   * \retval a new list.
   */
  gds_list_t * list_create(gds_list_cmp_f cmp,
			   gds_list_destroy_f destroy,
			   unsigned int resize_step);

  // -----[ list_destroy ]-------------------------------------------
  /**
   * Destroy a list.
   *
   * \param list_ref is a pointer to the list to be destroyed.
   */
  void list_destroy(gds_list_t ** list_ref);

  // -----[ list_index_of ]------------------------------------------
  int list_index_of(gds_list_t * list, void * item,
		    unsigned int * index_ref);
  // -----[ list_insert_at ]-----------------------------------------
  int list_insert_at(gds_list_t * list, unsigned int index,
		     void * item);
  // -----[ list_length ]--------------------------------------------
  unsigned int list_length(gds_list_t * list);
  // -----[ list_get_at ]--------------------------------------------
  void * list_get_at(gds_list_t * list, unsigned int index);
  // -----[ list_add ]-----------------------------------------------
  int list_add(gds_list_t * list, void * item);
  // -----[ list_remove_at ]-----------------------------------------
  int list_remove_at(gds_list_t * list, unsigned int index);
  // -----[ list_replace_at ]----------------------------------------
  int list_replace(gds_list_t * list, unsigned int index, void * item);
  // -----[ list_for_each ]------------------------------------------
  int list_for_each(gds_list_t * list, gds_list_foreach_f foreach,
		    void * ctx);
  // -----[ list_dup ]-----------------------------------------------
   gds_list_t * list_dup(gds_list_t * list, gds_list_dup_f dup);

#ifdef _cplusplus
}
#endif

#endif /* __GDS_LIST_H__ */
