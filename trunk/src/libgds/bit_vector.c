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
#include <string.h>

#include <libgds/bit_vector.h>
#include <libgds/array.h>
#include <libgds/memory.h>

struct _BitVector {
  SUInt32Array * puArray;
  uint32_t uSize;
};
#define BIT_VECTOR_SEGMENT_LEN 32

/** 
 * @file This data structure allows manipulation of user-defined length bit
 * vector. 
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
  uFilterSegment ^= (uCurrentBit << (BIT_VECTOR_SEGMENT_LEN - uBit - 1));
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

static int _bit_vector_segment_to_string(char ** ppStr, uint32_t uSegment, uint32_t * uCptBits)
{
  uint32_t uCpt;

  for (uCpt = 0; uCpt < BIT_VECTOR_SEGMENT_LEN; uCpt++) {
    (*ppStr)[uCpt] = (1 & (uSegment >> (BIT_VECTOR_SEGMENT_LEN - uCpt - 1))) + 48;
    (*uCptBits)--;
    if (!(*uCptBits))
      break;
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
  SEnumerator * pEnum;
  uint32_t uCptBits;
  uint32_t uSegment;

  if (!pBitVector)
    return NULL;

  /* Memory allocation of the string */
  pStr = MALLOC(pBitVector->uSize + 1);
  pStr[pBitVector->uSize] = '\0';
  pStrToRet = pStr;

  /* Browse each segments for stringization */
  uCptBits = pBitVector->uSize;
  pEnum = _array_get_enum((SArray*)pBitVector->puArray);
  while (enum_has_next(pEnum)) {
    uSegment = *(uint32_t*) enum_get_next(pEnum);
    _bit_vector_segment_to_string(&pStr, uSegment, &uCptBits);
  }
  return pStrToRet;
}

/**
 * @brief Tests the equality of two bit vectors
 *
 * @param pBitVector1 the first bit vector of the equality test
 * @param pBitVector2 the second bit vector of the equality test
 *
 */
int8_t bit_vector_equals(SBitVector * pBitVector1, SBitVector * pBitVector2)
{
  SEnumerator * pEnum1;
  SEnumerator * pEnum2;
  uint32_t uSegment1;
  uint32_t uSegment2;

  if (!pBitVector1 && !pBitVector2)
    return 1;
  if (!pBitVector1 || !pBitVector2)
    return 0;

  /* Tests the size of the bit vectors */
  if (pBitVector1->uSize != pBitVector2->uSize)
    return 0;

  pEnum1 = _array_get_enum( (SArray*)pBitVector1->puArray );
  pEnum2 = _array_get_enum( (SArray*)pBitVector2->puArray );
  while (enum_has_next(pEnum1)) {
    uSegment1 = *(uint32_t*)enum_get_next(pEnum1);
    uSegment2 = *(uint32_t*)enum_get_next(pEnum2);

    if (uSegment1 != uSegment2)
      return 0;
  }
  return 1;
}

/**
 * @brief Compare two bit vectors
 *
 * The comparison is performed with the following two hypothesis :
 *  - NULL is the smallest value possible
 *  - one bit vector is greater than another if its size is greater than the second whatever
 *  the value of both bit vectors.
 *
 * @param pBitVector1 the first bit vector of the comparison
 * @param pBitVector2 the second bit vector of the comparison
 *
 * @return 0 if both bit vectors are the same, 1 if the first vector is greater
 * than the second, -1 if the first vector is smaller than the second.
 */
int8_t bit_vector_comp(SBitVector * pBitVector1, SBitVector * pBitVector2)
{
  SEnumerator * pEnum1;
  SEnumerator * pEnum2;
  uint32_t uSegment1;
  uint32_t uSegment2;

  /* Tests the NULL value which is the smallest possible */
  if (!pBitVector1 && !pBitVector2)
    return 0;
  if (!pBitVector1 || !pBitVector2)
    return 0;

  /* Tests the size of the bit vectors */
  if (pBitVector1->uSize < pBitVector2->uSize)
    return 1;
  else if (pBitVector1->uSize > pBitVector2->uSize)
    return -1;

  pEnum1 = _array_get_enum( (SArray*)pBitVector1->puArray );
  pEnum2 = _array_get_enum( (SArray*)pBitVector2->puArray );
  while (enum_has_next(pEnum1)) {
    uSegment1 = *(uint32_t*)enum_get_next(pEnum1);
    uSegment2 = *(uint32_t*)enum_get_next(pEnum2);

    if (uSegment1 < uSegment2)
      return -1;
    else if (uSegment1 > uSegment2)
      return 1;
  }
  return 0;
}

/*********************************
 * Binary Operations
 ********************************/
typedef enum {
  BIT_VECTOR_AND = 1,
  BIT_VECTOR_OR,
  BIT_VECTOR_XOR
}EBitVectorOperation;

typedef struct _BitVectorOperation {
  EBitVectorOperation op;
  SBitVector * pBitVector;
  uint32_t uNumSegment;
}SBitVectorOperation;

static int8_t _bit_vector_binary_operation(const EBitVectorOperation op, 
					    uint32_t * uSegment1, 
					    uint32_t uSegment2)
{
  if (!uSegment1)
    return -1;

  switch (op) {
    case BIT_VECTOR_AND:
      (*uSegment1) &= uSegment2;
      break;
    case BIT_VECTOR_OR:
      (*uSegment1) |= uSegment2;
      break;
    case BIT_VECTOR_XOR:
      (*uSegment1) ^= uSegment2;
      break;
    default:
      return -1;
  }
  return 0;
}

static int _bit_vector_binary_operation_enum(const EBitVectorOperation op,
					      SBitVector * pBitVector1, 
					      SBitVector * pBitVector2)
{
  SEnumerator * pEnum1;
  SEnumerator * pEnum2;
  uint32_t uSegment1;
  uint32_t uSegment2;

  uint32_t uCptSegment = 0;


  if (pBitVector1->uSize != pBitVector2->uSize) 
    return -1;

  pEnum1 = _array_get_enum((SArray *) pBitVector1->puArray);
  pEnum2 = _array_get_enum((SArray *) pBitVector2->puArray);

  while (enum_has_next(pEnum1)) {
    uSegment1 = *(uint32_t*) enum_get_next(pEnum1);
    uSegment2 = *(uint32_t*) enum_get_next(pEnum2);

    if (_bit_vector_binary_operation(op, &uSegment1, uSegment2)) 
      return -1;

    _array_set_at((SArray*)pBitVector1->puArray, uCptSegment, &uSegment1);
    uCptSegment++;
  }
  return 0;
}

/**
 * @brief Performs an @em and operation on a bit vector.
 *
 * @param pBitVector1 the vector affected by the \em and operation
 * @param pBitVector2 the vector to and the first with.
 *
 * @return 0 if pBitVector1 has been anded with pBitVector2, else if one of the
 * two bit vectors is NULL, -1 is returned. -1 is also returned if the length
 * of the bit vectors aren't the same.
 *
 * @warning It is \em not possible to perform the @em and operation on bit
 * vectors of different lengths. (TODO?)
 */
int8_t bit_vector_and(SBitVector * pBitVector1, SBitVector * pBitVector2)
{
  if (!pBitVector1 || !pBitVector2)
    return -1;

  return _bit_vector_binary_operation_enum(BIT_VECTOR_AND, pBitVector1, pBitVector2);
}

/**
 * @brief Performs an @em or operation on a bit vector.
 *
 * @param pBitVector1 the vector affected by the \em or operation
 * @param pBitVector2 the vector to and the first with.
 *
 * @return 0 if pBitVector1 has been ored with pBitVector2, else if one of the
 * two bit vectors is NULL, -1 is returned. -1 is also returned if the length
 * of the bit vectors aren't the same.
 *
 * @warning It is \em not possible to perform the @em or operation on bit
 * vectors of different lengths. (TODO?)
 */
int8_t bit_vector_or(SBitVector * pBitVector1, SBitVector * pBitVector2)
{
  if (!pBitVector1 || !pBitVector2)
    return -1;

  return _bit_vector_binary_operation_enum(BIT_VECTOR_OR, pBitVector1, pBitVector2);
}

/**
 * @brief Performs an @em or operation on a bit vector.
 *
 * @param pBitVector1 the vector affected by the \em or operation
 * @param pBitVector2 the vector to and the first with.
 *
 * @return 0 if pBitVector1 has been ored with pBitVector2, else if one of the
 * two bit vectors is NULL, -1 is returned. -1 is also returned if the length
 * of the bit vectors aren't the same.
 *
 * @warning It is \em not possible to perform the @em or operation on bit
 * vectors of different lengths. (TODO?)
 */
int8_t bit_vector_xor(SBitVector * pBitVector1, SBitVector * pBitVector2)
{
  if (!pBitVector1 || !pBitVector2)
    return -1;

  return _bit_vector_binary_operation_enum(BIT_VECTOR_XOR, pBitVector1, pBitVector2);
}

/**
 * @brief Creates a bit vector from a string representation
 *
 * @param cBitVector string representation of a bit vector
 *
 * @return a new allocated bit vector, of size equal to the length of the
 * string and initialised from the value of each character of the string.
 *
 * @note Each character which is not equal to '0' are set to one in the bit
 * vector.
 *
 */
SBitVector * bit_vector_create_from_string(char * cBitVector)
{
  SBitVector * pBitVector;
  uint32_t uLen;
  uint32_t uCptBits;
 
  uLen = strlen(cBitVector);
  pBitVector = bit_vector_create(uLen);

  for (uCptBits = 0; uCptBits < uLen; uCptBits++) {
    if (cBitVector[uCptBits] != '0')
      bit_vector_set(pBitVector, uCptBits);
  }
  return pBitVector;
}

