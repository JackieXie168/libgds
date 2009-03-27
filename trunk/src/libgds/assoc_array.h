// ==================================================================
// @(#)assoc_array.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 05/01/2007
// $Id$
// ==================================================================

/**
 * \file
 * Provide data structure and functions to manage an associative array,
 * i.e. an array mapping a string key to a data pointer.
 *
 * Typical example:
 * \code
 * gds_enum_t * enu;
 * gds_assoc_array_t * array= assoc_array_create(NULL);
 * assoc_array_set(array, "foo", "bar");
 * assoc_array_set(array, "cat", "murphy");
 * enu= assoc_array_get_keys_enum(array);
 * while (enum_has_next(enu)) {
 *   key= (char *) enum_get_next(enu);
 *   fprintf(stdout, "key:\"%s\" => value:\"%s\"\n",
 *           key, (char*) assoc_array_get(array, key));
 *   fflush(stdout);
 * }
 * assoc_array_destroy(&array);
 *
 * \endcode
 */

#ifndef __GDS_ASSOC_ARRAY_H__
#define __GDS_ASSOC_ARRAY_H__

#include <libgds/array.h>
#include <libgds/enumerator.h>

typedef ptr_array_t gds_assoc_array_t;

/** Callback used to traverse an associative array. */
typedef int (*assoc_array_foreach_f)(const char * key, void * data,
				     void * ctx);
/** Callback used to free associated data in an associative array. */
typedef void (*assoc_array_destroy_f)(void * item);

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ assoc_array_create ]-------------------------------------
  /**
   * Create an associative array.
   *
   * \param destroy is an optional item destroy function (can be NULL).
   * \retval an associative array.
   */
  gds_assoc_array_t * assoc_array_create(assoc_array_destroy_f destroy);

  // -----[ assoc_array_destroy ]------------------------------------
  /**
   * Destroy an associative array.
   *
   * \param array_ref is a pointer to the associative array to be
   *   destroyed.
   */
  void assoc_array_destroy(gds_assoc_array_t ** array_ref);

  // -----[ assoc_array_length ]-------------------------------------
  /**
   * Get the length of an associative array.
   *
   * \param array is the associative array.
   * \retval the number of different keys in the associative array,
   *   i.e. its length.
   */
  unsigned int assoc_array_length(gds_assoc_array_t * array);

  // -----[ assoc_array_exists ]-------------------------------------
  /**
   * Test if a key exists in an associative array.
   *
   * \param array is the associative array.
   * \param key   is the searched key.
   * \retval 0 if the key does not exist,
   *   or != 0 otherwise.
   */
  int assoc_array_exists(gds_assoc_array_t * array, const char * key);

  // -----[ assoc_array_get ]----------------------------------------
  /**
   * Get the data associated to a key in an associative array.
   *
   * \param array is the associative array.
   * \param key   is the searched key.
   * \retval the associated data if the key exists,
   *   or NULL otherwise.
   */
  void * assoc_array_get(gds_assoc_array_t * array, const char * key);

  // -----[ assoc_array_set ]----------------------------------------
  /**
   * Set the data associated to a key in an associative array.
   *
   * If this key already exists, the previously associated data is
   * freed using the \c destroy function provided to
   * assoc_array_create. If no \c destroy function was  provided,
   * the associated data is not freed.
   *
   * \param array is the associative array.
   * \param key   is the key string.
   * \param data  is the data to associate with \a key.
   * \retval 0 in case of success,
   *   or <0 in case of failure.
   */
  int assoc_array_set(gds_assoc_array_t * array, const char * key,
		      const void * data);

  // -----[ assoc_array_for_each ]-----------------------------------
  /**
   * Traverse an associative array.
   *
   * Call the provided callback function \a foreach for each key in
   * the associative array.
   *
   * \param array   is the associative array.
   * \param foreach is the callback function.
   * \param ctx     is the callback context pointer (will be passed
   *   to the callback each time it is called.
   * \retval 0 in case of success (all calls to \a foreach succeeded),
   *   or -1 in case of failure (at least one call to \a foreach
   *   a negative result).
   */
  int assoc_array_for_each(gds_assoc_array_t * array,
			   assoc_array_foreach_f foreach,
			   void * ctx);

  // -----[ assoc_array_get_enum ]-----------------------------------
  /**
   * Get an enumeration to traverse an associative array.
   *
   * \param array        is the associative array.
   * \param key_or_value selects if the enumeration is for keys or
   *   values. If \a key_or_value is ASSOC_ARRAY_ENUM_KEYS, the
   *   enumeration will return all distinct keys. If \a key_or_value
   *   is ASSOC_ARRAY_ENUM_VALUES; the enumeration will return all
   *   values.
   * \retval en enumeration.
   *
   * \attention
   * If ASSOC_ARRAY_ENUM_VALUES is used, note that the returned
   * values may be reported multiple times if the same value is
   * associated to multiple distinct keys.
   */
  gds_enum_t * assoc_array_get_enum(gds_assoc_array_t * array,
				    int key_or_value);

#ifdef __cplusplus
}
#endif

#define ASSOC_ARRAY_ENUM_KEYS   0
#define ASSOC_ARRAY_ENUM_VALUES 1

/** Get an enumeration for the keys in an associative array.
 * \see assoc_array_get_enum */
#define assoc_array_get_keys_enum(A) \
  assoc_array_get_enum(A, ASSOC_ARRAY_ENUM_KEYS)

/** Get an enumeration for the values in an associative array.
 * \see assoc_array_get_enum */
#define assoc_array_get_values_enum(A) \
  assoc_array_get_enum(A, ASSOC_ARRAY_ENUM_VALUES)

#endif /* __GDS_ASSOC_ARRAY_H__ */
