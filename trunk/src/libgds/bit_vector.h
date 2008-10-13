// ==================================================================
// @(#)bit_vector.h
//
// @author Sebastien Tandel <sebastien [AT] tandel (dot) be>
// @date 15/03/2007
// $Id$
// ==================================================================

#ifndef __GDS_BIT_VECTOR_H__
#define __GDS_BIT_VECTOR_H__

#include <libgds/types.h>

typedef struct gds_bit_vector_t gds_bit_vector_t;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ bit_vector_create ]--------------------------------------
  gds_bit_vector_t * bit_vector_create(unsigned int size);
  // -----[ bit_vector_destroy ]-------------------------------------
  void bit_vector_destroy(gds_bit_vector_t ** vector);
  // -----[ bit_vector_set ]-----------------------------------------
  int8_t bit_vector_set(gds_bit_vector_t * vector,
			unsigned int index);
  // -----[ bit_vector_clear ]---------------------------------------
  int8_t bit_vector_clear(gds_bit_vector_t * vector,
			  unsigned int index);
  // -----[ bit_vector_get ]-----------------------------------------
  int8_t bit_vector_get(gds_bit_vector_t * vector,
			unsigned int index);
  // -----[ bit_vector_to_string ]-----------------------------------
  char * bit_vector_to_string(gds_bit_vector_t * vector);
  // -----[ bit_vector_cmp ]-----------------------------------------
  int8_t bit_vector_cmp(gds_bit_vector_t * vector1,
			gds_bit_vector_t * vector2);
  // -----[ bit_vector_equals ]--------------------------------------
  int8_t bit_vector_equals(gds_bit_vector_t * vector1,
			   gds_bit_vector_t * vector2);
  // -----[ bit_vector_and ]-----------------------------------------
  int8_t bit_vector_and(gds_bit_vector_t * vector1,
			gds_bit_vector_t * vector2);
  // -----[ bit_vector_or ]------------------------------------------
  int8_t bit_vector_or(gds_bit_vector_t * vector1,
		       gds_bit_vector_t * vector2);
  // -----[ bit_vector_xor ]-----------------------------------------
  int8_t bit_vector_xor(gds_bit_vector_t * vector1,
			gds_bit_vector_t * vector2);
  // -----[ bit_vector_from_string ]---------------------------------
  gds_bit_vector_t * bit_vector_from_string(const char * str);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_BIT_VECTOR_H__ */
