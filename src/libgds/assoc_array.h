// ==================================================================
// @(#)assoc_array.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 05/01/2007
// $Id$
// ==================================================================

#ifndef __GDS_ASSOC_ARRAY_H__
#define __GDS_ASSOC_ARRAY_H__

#include <libgds/array.h>
#include <libgds/enumerator.h>

typedef ptr_array_t gds_assoc_array_t;
typedef int (*assoc_array_foreach_f)(const char * key, void * data,
				     void * ctx);
typedef void (*assoc_array_destroy_f)(void * item);

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ assoc_array_create ]-------------------------------------
  gds_assoc_array_t * assoc_array_create(assoc_array_destroy_f destroy);
  // -----[ assoc_array_destroy ]------------------------------------
  void assoc_array_destroy(gds_assoc_array_t ** array_ref);
  // -----[ assoc_array_length ]-------------------------------------
  unsigned int assoc_array_length(gds_assoc_array_t * array);
  // -----[ assoc_array_exists ]-------------------------------------
  int assoc_array_exists(gds_assoc_array_t * array, const char * key);
  // -----[ assoc_array_get ]----------------------------------------
  void * assoc_array_get(gds_assoc_array_t * array, const char * key);
  // -----[ assoc_array_set ]----------------------------------------
  int assoc_array_set(gds_assoc_array_t * array, const char * key,
		      const void * data);
  // -----[ assoc_array_for_each ]-----------------------------------
  int assoc_array_for_each(gds_assoc_array_t * array,
			   assoc_array_foreach_f foreach,
			   void * ctx);
  // -----[ assoc_array_get_enum ]-----------------------------------
  gds_enum_t * assoc_array_get_enum(gds_assoc_array_t * array,
				    int key_or_value);

#ifdef __cplusplus
}
#endif

#define ASSOC_ARRAY_ENUM_KEYS   0
#define ASSOC_ARRAY_ENUM_VALUES 1

#define assoc_array_get_keys_enum(A) \
  assoc_array_get_enum(A, ASSOC_ARRAY_ENUM_KEYS)
#define assoc_array_get_values_enum(A) \
  assoc_array_get_enum(A, ASSOC_ARRAY_ENUM_VALUES)

#endif /* __GDS_ASSOC_ARRAY_H__ */
