// ==================================================================
// @(#)bit_vector.h
//
// @author Sebastien Tandel <sebastien [AT] tandel (dot) be>
// @date 15/03/2007
// @lastdate 15/03/2007
// ==================================================================

#ifndef __BIT_VECTOR_H__
#define __BIT_VECTOR_H__

#include <libgds/types.h>

typedef struct _BitVector SBitVector;

// ----- bit_vector_create -------------------------------------------
SBitVector * bit_vector_create(const uint32_t uSize);
// ----- bit_vector_destroy ------------------------------------------
void bit_vector_destroy(SBitVector ** pBitVector);
// ----- bit_vector_set ----------------------------------------------
int8_t bit_vector_set(SBitVector * pBitVector, const uint32_t uNumBit);
// ----- bit_vector_unset --------------------------------------------
int8_t bit_vector_unset(SBitVector * pBitVector, const uint32_t uNumBit);
// ----- bit_vector_get ----------------------------------------------
int8_t bit_vector_get(SBitVector * pBitVector, const uint32_t uNumBit);
// ----- bit_vector_to_string ----------------------------------------
char * bit_vector_to_string(SBitVector * pBitVector);
// ----- bit_vector_comp ---------------------------------------------
int8_t bit_vector_comp(SBitVector * pBitVector1, SBitVector * pBitVector2);
// ----- bit_vector_equals -------------------------------------------
int8_t bit_vector_equals(SBitVector * pBitVector1, SBitVector * pBitVector2);
// ----- bit_vector_and ----------------------------------------------
int8_t bit_vector_and(SBitVector * pBitVector1, SBitVector * pBitVector2);
// ----- bit_vector_or -----------------------------------------------
int8_t bit_vector_or(SBitVector * pBitVector1, SBitVector * pBitVector2);
// ----- bit_vector_xor ----------------------------------------------
int8_t bit_vector_xor(SBitVector * pBitVector1, SBitVector * pBitVector2);
// ----- bit_vector_from_string --------------------------------------
SBitVector * bit_vector_create_from_string(char * cBitVector);
#endif /* __BIT_VECTOR_H__ */
