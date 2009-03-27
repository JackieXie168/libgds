// ==================================================================
// @(#)sequence.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 23/11/2002
// $Id$
// ==================================================================

/**
 * \file
 * Provide data structure and functions to manipulate a sequence of
 * items.
 */

#ifndef __GDS_SEQUENCE_H__
#define __GDS_SEQUENCE_H__

/** Compare two items in the sequence. */
typedef int (*gds_seq_cmp_f)(const void * item1, const void * item2);

/** Destroy an item in the sequence. */
typedef void (*gds_seq_destroy_f)(void ** item_ref);

/** Callback used to traverse the whole sequence. */
typedef int (*gds_seq_foreach_f)(const void * item, void * ctx);

/** Copy an item. */
typedef void * (*gds_seq_copy_f)(void * item);

// -----[ gds_seq_t ]------------------------------------------------
typedef struct {
  unsigned int         size;
  void              ** items;
  gds_seq_cmp_f        cmp;
  gds_seq_destroy_f    destroy;
} gds_seq_t;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ sequence_create ]----------------------------------------
  /**
   * Create a sequence.
   *
   * \param cmp     is an item comparison function (can be NULL).
   * \param destroy is an item destruction function (can be NULL).
   */
  gds_seq_t * sequence_create(gds_seq_cmp_f cmp, gds_seq_destroy_f destroy);

  // -----[ sequence_destroy ]---------------------------------------
  /**
   * Destroy a sequence.
   *
   * If non-NULL, will call the destroy callback on each item.
   *
   * \param seq_ref is a pointer to the sequence.
   */
  void sequence_destroy(gds_seq_t ** seq_ref);

  // -----[ sequence_find_index ]------------------------------------
  /**
   * Find the index of an item in the sequence.
   *
   * The comparison callback is used to find the item. If the
   * comparison callback is NULL, the pointer value of item is
   * compared to the pointer values in the sequence.
   *
   * \param seq       is the target sequence.
   * \param item      is the searched item.
   * \param index_ref is the pointer to the returned index
   * \retval 0 if found,
   *         <0 otherwise
   */
  int sequence_index_of(gds_seq_t * seq, void * item,
			unsigned int * index_ref);

  // -----[ sequence_insert_at ]-------------------------------------
  /**
   * Insert an item at a given position in the sequence.
   *
   * \param seq   is the target sequence.
   * \param index is the insertion position [0, size].
   * \param item  is the item to be inserted.
   */
  void sequence_insert_at(gds_seq_t * seq, unsigned int index,
			  void * item);

  // -----[ sequence_add ]-------------------------------------------
  /**
   * Add an item to the sequence.
   *
   * The item is inserted at the end of the sequence.
   *
   * \param seq  is the target sequence.
   * \param item is the item to be inserted.
   */
  void sequence_add(gds_seq_t * seq, void * item);

  // -----[ sequence_remove ]----------------------------------------
  /**
   * Remove an item from the sequence.
   *
   * The comparison callback is used to find the item. If the
   * comparison callback is NULL, the pointer value of item is
   * compared to the pointer values in the sequence. The destroy
   * callback is called to free the removed item.
   *
   * \param seq  is the target sequence.
   * \param item is the item to be removed.
   * \retval 0 in case of success,
   *         <0 otherwise
   */
  int sequence_remove(gds_seq_t * seq, void * item);

  // ----- sequence_remove_at -----------------------------------------
  /**
   * Remove an item at a given position in the sequence.
   *
   * \param seq   is the target sequence.
   * \param index is the location of the item to be removed.
   */
  void sequence_remove_at(gds_seq_t * seq, unsigned int index);

  // -----[ sequence_for_each ]--------------------------------------
  /**
   * Call a specific function for each item in the sequence.
   *
   * \param seq     is the target sequence.
   * \param foreach is the callback function.
   * \param ctx     is the callback context pointer.
   * \retval 0 in case of success,
   *         <0 otherwise
   */
  int sequence_for_each(gds_seq_t * seq, gds_seq_foreach_f foreach,
			void * ctx);

  // -----[ sequence_copy ]----------------------------------------------
  /**
   * Duplicate a sequence and its content.
   *
   * The items added to the new sequence are produced by the provided
   * copy function.
   *
   * \param seq  is the original sequence.
   * \param copy is the copy function.
   * \retval a new sequence
   */
  gds_seq_t * sequence_copy(gds_seq_t * seq,
			    gds_seq_copy_f copy);
  
#ifdef __cplusplus
}
#endif

#endif /* __GDS_SEQUENCE_H__ */
