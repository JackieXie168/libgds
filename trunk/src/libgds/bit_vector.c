// ==================================================================
// @(#)bloom_vector.c
//
// @author Sebastien Tandel <sebastien [AT] tandel (dot) be>
// @date 15/03/2007
// @lastdate 15/03/2007
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <libgds/bit_vector.h>
#include <libgds/array.h>
#include <libgds/memory.h>

struct _BitVector {
  SUInt32Array * puArray;
  uint32_t uSize;
};
#define BIT_VECTOR_SEGMENT_LEN 32

/** 
 * The bit vector is an array composed of elements of 32 bits.
 */

// ----- bit_vector_create -------------------------------------------
/**
 * \brief Creates a bit vector of size uSize.  
 *
 * @param uSize the size of the bit vector
 *
 * @return a bit vector of size uSize
 */
SBitVector * bit_vector_create(const uint32_t uSize)
{
  SBitVector * pBitVector = MALLOC(sizeof(SBitVector));
  uint32_t uZero = 0;
  uint32_t uCpt;
  uint32_t uNbrSegments;

  pBitVector->uSize = uSize;
  pBitVector->puArray = uint32_array_create(0);
  uNbrSegments = uSize / BIT_VECTOR_SEGMENT_LEN;
  if (uSize % BIT_VECTOR_SEGMENT_LEN) {
    uNbrSegments++;
  }
  _array_set_length((SArray*)pBitVector->puArray, uNbrSegments);
  for (uCpt = 0; uCpt < _array_length((SArray*)pBitVector->puArray); uCpt++) {
    _array_set_at((SArray*)pBitVector->puArray, uCpt, &uZero);
  }
  return pBitVector;
}

// ----- bit_vector_destroy ------------------------------------------
/**
 * \brief destroys the bit vector pBitVector
 *
 * @param pBitVector a bit vector
 *
 */
void bit_vector_destroy(SBitVector ** pBitVector)
{
  if (*pBitVector) {
    if ( (*pBitVector)->puArray ) {
      uint32_array_destroy( &(*pBitVector)->puArray );
    }
    FREE( (*pBitVector) );
    *pBitVector = NULL;
  }
}

// ----- bit_vector_set ----------------------------------------------
/**
 * \brief set to 1 a specific bit of a bit vector.
 *
 * @param pBitVector a bit vector
 * @param uNumBit the number of the bit to set to 1 in the bit vector
 *
 * @return 0 if the bit is set. -1 if pBitVector is NULL or if uNumBit is
 * greater than the size of the bit vector.
 */
int8_t bit_vector_set(SBitVector * pBitVector, const uint32_t uNumBit)
{
  uint32_t uNumVector;
  uint32_t uFilterSegment;
  uint8_t uBit;

  if (!pBitVector || pBitVector->uSize <= uNumBit)
    return -1;

  /* Retrieve the segment which contains the bit to change */
  uNumVector = uNumBit/BIT_VECTOR_SEGMENT_LEN;
  _array_get_at((SArray*)pBitVector->puArray, uNumVector, &uFilterSegment);

  /* Change the value of the concerned bit */
  uBit = uNumBit%BIT_VECTOR_SEGMENT_LEN;
  uFilterSegment |= (1 << (BIT_VECTOR_SEGMENT_LEN - uBit - 1));
  _array_set_at((SArray*)pBitVector->puArray, uNumVector, &uFilterSegment);

  return 0;
}

// ----- bit_vector_unset --------------------------------------------
/**
 * \brief set to 0 a specific bit of a bit vector.
 *
 * @param pBitVector a bit vector
 * @param uNumBit the number of the bit to set to 0 in the bit vector
 *
 * @return 0 if the bit is set to 0. -1 if pBitVector is NULL or uNumVector is
 * greater than the size of the bit vector.
 */
int8_t bit_vector_unset(SBitVector * pBitVector, const uint32_t uNumBit)
{
  uint32_t uNumVector;
  uint32_t uFilterSegment;
  uint32_t uCurrentBit;
  uint8_t uBit;


  if (!pBitVector || pBitVector->uSize <= uNumBit)
    return -1;

  /* Retrieve the segment which contains the bit to change */
  uNumVector = uNumBit/BIT_VECTOR_SEGMENT_LEN;
  _array_get_at((SArray*)pBitVector->puArray, uNumVector, &uFilterSegment);

  /* Change the value of the concerned bit */
  uBit = uNumBit%BIT_VECTOR_SEGMENT_LEN;
  uCurrentBit = ((uFilterSegment << uBit) >> (BIT_VECTOR_SEGMENT_LEN-1));
  uFilterSegment ^= (uCurrentBit << (BIT_VECTOR_SEGMENT_LEN - uBit));
  _array_set_at((SArray*)pBitVector->puArray, uNumVector, &uFilterSegment);

  return 0;
}

// ----- bit_vector_get ----------------------------------------------
/**
 * \brief get the value of a specific bit of a bit vector
 *
 * @param pBitVector a bit vector
 * @param uNumBit the number of the specific bit to get in the bit vector
 *
 * @return -1 if pBitVector is NULL or uNumVector is greater tha the size of
 * the bit vector else the value of the bit in the bit vector (0/1).
 */
int8_t bit_vector_get(SBitVector * pBitVector, uint32_t uNumBit)
{
  uint32_t uNumVector;
  uint32_t uFilterSegment;
  uint8_t uBit;
    
  if (!pBitVector || pBitVector->uSize <= uNumBit)
    return -1;

  /* Retrieve the segment which contains the bit to change */
  uNumVector = uNumBit/BIT_VECTOR_SEGMENT_LEN;
  _array_get_at((SArray*)pBitVector->puArray, uNumVector, &uFilterSegment);

  /* Change the value of the concerned bit */
  uBit = uNumBit%BIT_VECTOR_SEGMENT_LEN;
  return ((uFilterSegment << uBit) >> (BIT_VECTOR_SEGMENT_LEN-1));
}

static int _bit_vector_to_string_for_each(void * pItem, void * pCtx)
{
  char ** ppStr = (char **)pCtx;
  uint32_t uItem = *(uint32_t *)pItem;
  uint32_t uCptBits;

  for (uCptBits = 0; uCptBits < BIT_VECTOR_SEGMENT_LEN; uCptBits++) {
    (*ppStr)[uCptBits] = (1 & (uItem >> (BIT_VECTOR_SEGMENT_LEN - uCptBits - 1))) + 48;
  //  printf("c : %c\n", (*ppStr)[uCptBits]);
  }
  (*ppStr) += BIT_VECTOR_SEGMENT_LEN;
  return 0;
}

// ----- bit_vector_to_string ----------------------------------------
/**
 * \brief Creates a string representation of a bit vector
 *
 * @param pBitVector a bit vector
 *
 * @return the string representation of the bit vector.
 */
char * bit_vector_to_string(SBitVector * pBitVector)
{
  char * pStr;
  char * pStrToRet;

  if (!pBitVector)
    return NULL;

  pStr = MALLOC(pBitVector->uSize + 1);
  pStr[pBitVector->uSize-1] = '\0';
  pStrToRet = pStr;
  _array_for_each( (SArray*)pBitVector->puArray, 
		    _bit_vector_to_string_for_each, 
		    &pStr);
  return pStrToRet;
}

