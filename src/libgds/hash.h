// ==================================================================
// @(#)hash.h
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 03/12/2004
// $Id$
// ==================================================================

/**
 * \file
 * Provide data structures and functions to manage hash-sets.
 */

#ifndef __GDS_HASH_H__
#define __GDS_HASH_H__

#include <libgds/enumerator.h>
#include <libgds/types.h>

typedef int      (*gds_hash_cmp_f)     (const void * item1,
				        const void * item2,
				        unsigned int item_size);
typedef void     (*gds_hash_destroy_f) (void * item);
typedef uint32_t (*gds_hash_compute_f) (const void * item,
				        unsigned int hash_size);
typedef int      (*gds_hash_foreach_f) (void * item, void * ctx);

typedef struct gds_hash_set_t gds_hash_set_t;

/** Operation is successful. */
#define HASH_SUCCESS        0
/** An item was unreferenced (not removed). */
#define HASH_SUCCESS_UNREF  1
/** No item was found. */
#define HASH_ERROR_NO_MATCH -1

#ifdef __cplusplus
extern "C" {
#endif

  ///////////////////////////////////////////////////////////////////
  // HASH MAP
  ///////////////////////////////////////////////////////////////////

  /* To be provided... */


  ///////////////////////////////////////////////////////////////////
  // HASH SET
  ///////////////////////////////////////////////////////////////////

  // -----[ hash_set_create ]----------------------------------------
  /**
   * Create a hash-set.
   */
  gds_hash_set_t * hash_set_create(unsigned int size,
				   float resize_threshold,
				   gds_hash_cmp_f cmp,
				   gds_hash_destroy_f destroy,
				   gds_hash_compute_f compute);

  // -----[ hash_set_destroy ]---------------------------------------
  /**
   * Destroy a hash-set.
   *
   * \param hash_ref is a pointer to the hash-set to be destroyed.
   */
  void hash_set_destroy(gds_hash_set_t ** hash_ref);

  // -----[ hash_set_add ]-------------------------------------------
  /**
   * Add an element to a hash-set.
   *
   * Add a new element to the hash-set. If the element already exists,
   * i.e. if it is the same as an already existing element according
   * to the comparison function, then the reference count of the
   * existing element is incremented.
   *
   * \param hash is the hash-set.
   * \param item is the item to be added.
   * \retval 
   *
   *   \li element did not exist:
   *   The pointer to the element in the hash-table is returned. If the
   *   return value is equal to \p item, that means that a new element
   *   was added.
   *   \li element already exists:
   *   The pointer to the existing element is returned. If the return
   *   value is different from \p item, that means that an equivalent
   *   element was already in the hash table.
   */
  void * hash_set_add(gds_hash_set_t * hash, void * item);

  // -----[ hash_set_remove ]----------------------------------------
  /**
   * Remove an element from a hash-set.
   *
   * \param hash is the hash-set.
   * \param item is the item to be removed.
   * \retval
   *
   * \li HASH_ERROR_NO_MATCH if no elt was deleted or unreferenced
   * \li HASH_SUCCESS_UNREF  if one element was unreferenced 
   * \li HASH_SUCCESS        if one element was deleted (refcount became 0)
   */
  int hash_set_remove(gds_hash_set_t * hash, void * item);

  // -----[ hash_set_search ]----------------------------------------
  /**
   * Lookup an item in a hash-set.
   *
   * \retval the searched item if it exists,
   *   or NULL if does not exist.
   */
  void * hash_set_search(const gds_hash_set_t * hash, void * item);

  // -----[ hash_set_get_refcnt ]------------------------------------
  unsigned int hash_set_get_refcnt(const gds_hash_set_t * hash,
				   void * item);
  // -----[ hash_set_for_each ]--------------------------------------
  int hash_set_for_each(const gds_hash_set_t * hash,
			gds_hash_foreach_f foreach,
			void * ctx);
  // -----[ hash_set_for_each_key ]----------------------------------
  int hash_set_for_each_key(const gds_hash_set_t * hash,
			    gds_hash_foreach_f foreach, 
			    void * ctx);
  // -----[ hash_set_get_enum ]--------------------------------------
  gds_enum_t * hash_set_get_enum(gds_hash_set_t * hash);
  // -----[ hash_set_dump ]------------------------------------------
  void hash_dump(const gds_hash_set_t * hash);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_HASH_H__ */
