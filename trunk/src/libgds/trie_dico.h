// ==================================================================
// @(#)tri_dico.h
//
// Dictionnaire  compact trie_dico implementation.
//
// @author Stefan Beauport (stefan.beauport@umons.ac.be)
// @date 19/08/2010
// $Id$
// ==================================================================


#ifndef __GDS_TRIE_DICO_H__
#define __GDS_TRIE_DICO_H__

#include <libgds/array.h>
#include <libgds/stream.h>

  /** Trie dico key data type. */
typedef char *  trie_dico_key_t;

#define TRIE_DICO_SUCCESS          0
#define TRIE_DICO_ERROR_DUPLICATE -1
#define TRIE_DICO_ERROR_NO_MATCH  -2

#define TRIE_DICO_INSERT_OR_REPLACE 1

#define TRIE_DICO_KEY_SIZE (sizeof(trie_dico_key_t)*8)

/** Callback function to traverse whole trie_dico. */
typedef int (*gds_trie_dico_foreach_f) (trie_dico_key_t key,
				    void * data, void * ctx);

/** Callback function to destroy a trie_dico item. */
typedef void (*gds_trie_dico_destroy_f) (void ** data);

// -----[ gds_trie_dico_t ]-----------------------------------------------
/**
 * Trie data structure.
 */
typedef struct gds_trie_dico_t {
  struct _trie_dico_item_t * root;
  gds_trie_dico_destroy_f    destroy;
} gds_trie_dico_t;

#ifdef	__cplusplus
extern "C" {
#endif

  // -----[ trie_dico_create ]--------------------------------------------
  /**
   * Create a trie_dico.
   *
   * \param destroy is an optional item destroy callback function
   *   (can be NULL).
   * \retval a new trie_dico.
   */
  gds_trie_dico_t * trie_dico_create(gds_trie_dico_destroy_f destroy);

  // -----[ trie_dico_destroy ]-------------------------------------------
  /**
   * Destroy a trie_dico.
   *
   * If a destroy callback function was assigned to \c trie_dico_create,
   * The \c trie_dico_destroy function will call this callback for each
   * item in the trie_dico.
   *
   * \param trie_dico_ref is a pointer to the trie_dico to be destroyed.
   */
  void trie_dico_destroy(gds_trie_dico_t ** trie_dico_ref);

  // -----[ trie_dico_find_exact ]----------------------------------------
  /**
   * Perform an exact match lookup in a trie_dico.
   *
   * \param trie_dico    is the trie_dico.
   * \param key     is the searched key.
   * \param key_len is the length of the searched key.
   * \retval the pointer associated to the given key if it exists,
   *   or NULL otherwise.
   */
  void * trie_dico_find_exact(gds_trie_dico_t * trie_dico,
			      trie_dico_key_t key);

  // -----[ trie_dico_find_best ]-----------------------------------------
  /**
   * Perform a best match lookup in a trie_dico.
   *
   * The best match is also known as a longest-match.
   *
   * \param trie_dico is the trie_dico.
   * \param key is the searched key.
   * \param key_len is the length of the searched key.
   * \retval the pointer associated to the best matching key if it
   *   exists, or NULL otherwise.
   */
  void * trie_dico_find_best(gds_trie_dico_t * trie_dico, trie_dico_key_t key);

  // -----[ trie_dico_insert ]--------------------------------------------
  /**
   * Insert data in a trie_dico.
   *
   * \param trie_dico    is the trie_dico.
   * \param key     is the insertion key.
   * \param key_len is the length of the key.
   * \param data    is the data pointer associated to the key.
   * \param replace selects wether or not the new data must replace
   *   the old one.
   * \retval TRIE_DICO_SUCCESS if the key could be successfully inserted,
   *   or TRIE_DICO_ERROR_DUPLICATE if replacement was not allowed and the
   *   key already exists.
   */
  int trie_dico_insert(gds_trie_dico_t * trie_dico, trie_dico_key_t key,
		       void * data, int replace);

  // -----[ trie_dico_remove ]--------------------------------------------
  /**
   * Remove a key from a trie_dico.
   *
   * \param trie_dico    is the trie_dico.
   * \param key     is the key to be removed.
   * \param key_len is the length of the key.
   * \retval 0 in case of success (key existed),
   *   or <0 in case of failure (key does not exist).
   */
  int trie_dico_remove(gds_trie_dico_t * trie_dico, trie_dico_key_t key);

  // -----[ trie_dico_replace ]-------------------------------------------
  /**
   * Replace an existing key in a trie_dico.
   *
   * \param trie_dico    is the trie_dico.
   * \param key     is the key to be replaced.
   * \param key_len is the length of the key.
   * \param data    is the new data.
   * \retval TRIE_DICO_SUCCESS in case the key could be replaced (it
   *   existed), or TRIE_DICO_ERROR_NO_MATCH if the key could not be
   *   found.
   */
  int trie_dico_replace(gds_trie_dico_t * trie_dico, trie_dico_key_t key,
                        void * data);

  // -----[ trie_dico_for_each ]------------------------------------------
  /**
   * Traverse a whole trie_dico.
   *
   * For each non empty node in the trie_dico, the provided \a foreach
   * callback function will be called.
   *
   * \param trie_dico is the trie_dico.
   * \param foreach is the callback function.
   * \param ctx is the callback context pointer. This pointer will
   *   be passed to the \a foreach callback function each time it is
   *   called.
   * \retval 0 in case all calls to \a foreach succeeded, or <0
   *   if at least one call th \a foreach failed (returned <0).
   */
  int trie_dico_for_each(gds_trie_dico_t * trie_dico,
			 gds_trie_dico_foreach_f foreach,
			 void * ctx);

  // -----[ trie_dico_get_array ]-----------------------------------------
  /**
   * Return an array with the items in the trie_dico.
   */
  ptr_array_t * trie_dico_get_array(gds_trie_dico_t * trie_dico);

  // -----[ trie_dico_get_enum ]------------------------------------------
  /**
   * Return an enumeration for items in the trie_dico.
   */
  gds_enum_t * trie_dico_get_enum(gds_trie_dico_t * trie_dico);

  // -----[ trie_dico_num_nodes ]-----------------------------------------
  /**
   * Return the number of nodes in the trie_dico.
   *
   * \param trie_dico is the trie_dico.
   * \param with_data selects wether or not the function will count
   *   the empty nodes. If \a with_data equals 1, only the nodes
   *   with data are counted. Otherwise, all the nodes are counted.
   * \retval the number of nodes in the trie_dico.
   */
  int trie_dico_num_nodes(gds_trie_dico_t * trie_dico, int with_data);

  // -----[ trie_dico_to_graphviz ]-----------------------------------------
  /**
   * \internal
   */
  void trie_dico_to_graphviz(gds_stream_t * stream,
			     gds_trie_dico_t * trie_dico);

#ifdef __cplusplus
}
#endif

#endif	/* __GDS_TRIE_DICO_H__ */

