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

typedef struct gds_bit_vector_t SBitVector;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ bit_vector_create ]--------------------------------------
  SBitVector * bit_vector_create(const uint32_t size);
  // ----- bit_vector_destroy ------------------------------------------
  void bit_vector_destroy(SBitVector ** vector);
  // ----- bit_vector_set ----------------------------------------------
  int8_t bit_vector_set(SBitVector * vector, const uint32_t uNumBit);
  // ----- bit_vector_unset --------------------------------------------
  int8_t bit_vector_unset(SBitVector * vector, const uint32_t uNumBit);
  // ----- bit_vector_get ----------------------------------------------
  int8_t bit_vector_get(SBitVector * vector, const uint32_t uNumBit);
  // ----- bit_vector_to_string ----------------------------------------
  char * bit_vector_to_string(SBitVector * vector);
  // ----- bit_vector_comp ---------------------------------------------
  int8_t bit_vector_comp(SBitVector * vector1, SBitVector * vector2);
  // ----- bit_vector_equals -------------------------------------------
  int8_t bit_vector_equals(SBitVector * vector1, SBitVector * vector2);
  // ----- bit_vector_and ----------------------------------------------
  int8_t bit_vector_and(SBitVector * vector1, SBitVector * vector2);
  // ----- bit_vector_or -----------------------------------------------
  int8_t bit_vector_or(SBitVector * vector1, SBitVector * vector2);
  // ----- bit_vector_xor ----------------------------------------------
  int8_t bit_vector_xor(SBitVector * vector1, SBitVector * vector2);
  // ----- bit_vector_from_string --------------------------------------
  SBitVector * bit_vector_create_from_string(char * cBitVector);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_BIT_VECTOR_H__ */
