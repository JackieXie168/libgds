// ==================================================================
// @(#)bit_vector.c
//
// @author Sebastien Tandel <sebastien [AT] tandel (dot) be>
// @date 15/03/2007
// $Id$
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

typedef struct gds_bit_vector_t {
  unsigned int     size;
  uint32_array_t * array;
} gds_bit_vector_t;
#define BIT_VECTOR_SEGMENT_LEN 32

/** 
 * @file This data structure allows manipulation of user-defined length bit
 * vector. 
 */

// -----[ bit_vector_create ]----------------------------------------
/**
 * \brief Creates a bit vector of size size.  
 *
 * @param size the size of the bit vector
 *
 * @return a bit vector of size size
 */
gds_bit_vector_t * bit_vector_create(const uint32_t size)
{
  gds_bit_vector_t * vector= MALLOC(sizeof(gds_bit_vector_t));
  unsigned int index;
  unsigned int num_segments;

  num_segments= size / BIT_VECTOR_SEGMENT_LEN;
  if (size % BIT_VECTOR_SEGMENT_LEN)
    num_segments++;

  vector->size= size;
  vector->array= uint32_array_create(0);
  uint32_array_set_size(vector->array, num_segments);
  for (index = 0; index < uint32_array_size(vector->array); index++)
    vector->array->data[index]= 0;
  return vector;
}

// ----- bit_vector_destroy ------------------------------------------
/**
 * \brief destroys the bit vector vector
 *
 * @param vector a bit vector
 *
 */
void bit_vector_destroy(gds_bit_vector_t ** vector)
{
  if (*vector) {
    if ( (*vector)->array ) {
      uint32_array_destroy( &(*vector)->array );
    }
    FREE( (*vector) );
    *vector = NULL;
  }
}

// ----- bit_vector_set ----------------------------------------------
/**
 * \brief set to 1 a specific bit of a bit vector.
 *
 * @param vector a bit vector
 * @param uNumBit the number of the bit to set to 1 in the bit vector
 *
 * @return 0 if the bit is set. -1 if vector is NULL or if uNumBit is
 * greater than the size of the bit vector.
 */
int8_t bit_vector_set(gds_bit_vector_t * vector, const uint32_t uNumBit)
{
  uint32_t uNumVector;
  uint32_t uFilterSegment;
  uint8_t uBit;

  if ((vector == NULL) ||
      (vector->size <= uNumBit))
    return -1;

  /* Retrieve the segment which contains the bit to change */
  uNumVector = uNumBit/BIT_VECTOR_SEGMENT_LEN;
  uFilterSegment= vector->array->data[uNumVector];

  /* Change the value of the concerned bit */
  uBit = uNumBit%BIT_VECTOR_SEGMENT_LEN;
  uFilterSegment |= (1 << (BIT_VECTOR_SEGMENT_LEN - uBit - 1));
  vector->array->data[uNumVector]= uFilterSegment;

  return 0;
}

// ----- bit_vector_unset --------------------------------------------
/**
 * \brief set to 0 a specific bit of a bit vector.
 *
 * @param vector a bit vector
 * @param uNumBit the number of the bit to set to 0 in the bit vector
 *
 * @return 0 if the bit is set to 0. -1 if vector is NULL or uNumVector is
 * greater than the size of the bit vector.
 */
int8_t bit_vector_unset(gds_bit_vector_t * vector, const uint32_t uNumBit)
{
  uint32_t uNumVector;
  uint32_t uFilterSegment;
  uint32_t uCurrentBit;
  uint8_t uBit;


  if ((vector == NULL) ||
      (vector->size <= uNumBit))
    return -1;

  /* Retrieve the segment which contains the bit to change */
  uNumVector = uNumBit/BIT_VECTOR_SEGMENT_LEN;
  uFilterSegment= vector->array->data[uNumVector];

  /* Change the value of the concerned bit */
  uBit = uNumBit%BIT_VECTOR_SEGMENT_LEN;
  uCurrentBit = ((uFilterSegment << uBit) >> (BIT_VECTOR_SEGMENT_LEN-1));
  uFilterSegment ^= (uCurrentBit << (BIT_VECTOR_SEGMENT_LEN - uBit - 1));
  vector->array->data[uNumVector]= uFilterSegment;

  return 0;
}

// ----- bit_vector_get ----------------------------------------------
/**
 * \brief get the value of a specific bit of a bit vector
 *
 * @param vector a bit vector
 * @param uNumBit the number of the specific bit to get in the bit vector
 *
 * @return -1 if vector is NULL or uNumVector is greater tha the size of
 * the bit vector else the value of the bit in the bit vector (0/1).
 */
int8_t bit_vector_get(gds_bit_vector_t * vector, uint32_t uNumBit)
{
  uint32_t uNumVector;
  uint32_t uFilterSegment;
  uint8_t uBit;
    
  if ((vector == NULL) ||
      (vector->size <= uNumBit))
    return -1;

  /* Retrieve the segment which contains the bit to change */
  uNumVector = uNumBit/BIT_VECTOR_SEGMENT_LEN;
  uFilterSegment= vector->array->data[uNumVector];

  /* Change the value of the concerned bit */
  uBit = uNumBit%BIT_VECTOR_SEGMENT_LEN;
  return ((uFilterSegment << uBit) >> (BIT_VECTOR_SEGMENT_LEN-1));
}

static int _bit_vector_segment_to_string(char ** ppStr, uint32_t segment, uint32_t * uCptBits)
{
  uint32_t uCpt;

  for (uCpt = 0; uCpt < BIT_VECTOR_SEGMENT_LEN; uCpt++) {
    (*ppStr)[uCpt] = (1 & (segment >> (BIT_VECTOR_SEGMENT_LEN - uCpt - 1))) + 48;
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
 * @param vector a bit vector
 *
 * @return the string representation of the bit vector.
 */
char * bit_vector_to_string(gds_bit_vector_t * vector)
{
  char * pStr;
  char * pStrToRet;
  uint32_t uCptBits;
  uint32_t segment;
  unsigned int index;

  if (!vector)
    return NULL;

  /* Memory allocation of the string */
  pStr= MALLOC(vector->size + 1);
  pStr[vector->size] = '\0';
  pStrToRet= pStr;

  /* Browse each segments for stringization */
  uCptBits= vector->size;
  for (index= 0; index < uint32_array_size(vector->array); index++) {
    segment= vector->array->data[index];
    _bit_vector_segment_to_string(&pStr, segment, &uCptBits);
  }
  return pStrToRet;
}

/**
 * @brief Tests the equality of two bit vectors
 *
 * @param vector1 the first bit vector of the equality test
 * @param vector2 the second bit vector of the equality test
 *
 */
int8_t bit_vector_equals(gds_bit_vector_t * vector1, gds_bit_vector_t * vector2)
{
  uint32_t segment1;
  uint32_t segment2;
  unsigned int index;

  if (!vector1 && !vector2)
    return 1;
  if (!vector1 || !vector2)
    return 0;

  /* Tests the size of the bit vectors */
  if (vector1->size != vector2->size)
    return 0;

  for (index= 0; index < uint32_array_size(vector1->array); index++) {
    segment1= vector1->array->data[index];
    segment2= vector2->array->data[index];

    if (segment1 != segment2)
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
 * @param vector1 the first bit vector of the comparison
 * @param vector2 the second bit vector of the comparison
 *
 * @return 0 if both bit vectors are the same, 1 if the first vector is greater
 * than the second, -1 if the first vector is smaller than the second.
 */
int8_t bit_vector_comp(gds_bit_vector_t * vector1, gds_bit_vector_t * vector2)
{
  uint32_t segment1;
  uint32_t segment2;
  unsigned int index;

  /* Tests the NULL value which is the smallest possible */
  if (!vector1 && !vector2)
    return 0;
  if (!vector1 || !vector2)
    return 0;

  /* Tests the size of the bit vectors */
  if (vector1->size < vector2->size)
    return 1;
  else if (vector1->size > vector2->size)
    return -1;

  for (index= 0; index < uint32_array_size(vector1->array); index++) {
    segment1= vector1->array->data[index];
    segment2= vector2->array->data[index];

    if (segment1 < segment2)
      return -1;
    else if (segment1 > segment2)
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
  gds_bit_vector_t * vector;
  uint32_t uNumSegment;
}gds_bit_vector_tOperation;

static inline
int _bit_vector_segment_binary_op(const EBitVectorOperation op, 
				  uint32_t * segment1, 
				  uint32_t segment2)
{
  if (!segment1)
    return -1;

  switch (op) {
    case BIT_VECTOR_AND:
      (*segment1) &= segment2;
      break;
    case BIT_VECTOR_OR:
      (*segment1) |= segment2;
      break;
    case BIT_VECTOR_XOR:
      (*segment1) ^= segment2;
      break;
    default:
      return -1;
  }
  return 0;
}

static inline int _bit_vector_binary_op(const EBitVectorOperation op,
					gds_bit_vector_t * vector1, 
					gds_bit_vector_t * vector2)
{
  uint32_t segment1;
  uint32_t segment2;
  unsigned int index;

  uint32_t uCptSegment = 0;

  if (vector1->size != vector2->size)
    return -1;

  for (index= 0; index < uint32_array_size(vector1->array); index++) {
    segment1= vector1->array->data[index];
    segment2= vector2->array->data[index];

    if (_bit_vector_segment_binary_op(op, &segment1, segment2)) 
      return -1;

    vector1->array->data[uCptSegment]= segment1;
    uCptSegment++;
  }
  return 0;
}

/**
 * @brief Performs an @em and operation on a bit vector.
 *
 * @param vector1 the vector affected by the \em and operation
 * @param vector2 the vector to and the first with.
 *
 * @return 0 if vector1 has been anded with vector2, else if one of the
 * two bit vectors is NULL, -1 is returned. -1 is also returned if the length
 * of the bit vectors aren't the same.
 *
 * @warning It is \em not possible to perform the @em and operation on bit
 * vectors of different lengths. (TODO?)
 */
int8_t bit_vector_and(gds_bit_vector_t * vector1, gds_bit_vector_t * vector2)
{
  if (!vector1 || !vector2)
    return -1;

  return _bit_vector_binary_op(BIT_VECTOR_AND, vector1, vector2);
}

/**
 * @brief Performs an @em or operation on a bit vector.
 *
 * @param vector1 the vector affected by the \em or operation
 * @param vector2 the vector to and the first with.
 *
 * @return 0 if vector1 has been ored with vector2, else if one of the
 * two bit vectors is NULL, -1 is returned. -1 is also returned if the length
 * of the bit vectors aren't the same.
 *
 * @warning It is \em not possible to perform the @em or operation on bit
 * vectors of different lengths. (TODO?)
 */
int8_t bit_vector_or(gds_bit_vector_t * vector1, gds_bit_vector_t * vector2)
{
  if (!vector1 || !vector2)
    return -1;

  return _bit_vector_binary_op(BIT_VECTOR_OR, vector1, vector2);
}

/**
 * @brief Performs an @em or operation on a bit vector.
 *
 * @param vector1 the vector affected by the \em or operation
 * @param vector2 the vector to and the first with.
 *
 * @return 0 if vector1 has been ored with vector2, else if one of the
 * two bit vectors is NULL, -1 is returned. -1 is also returned if the length
 * of the bit vectors aren't the same.
 *
 * @warning It is \em not possible to perform the @em or operation on bit
 * vectors of different lengths. (TODO?)
 */
int8_t bit_vector_xor(gds_bit_vector_t * vector1, gds_bit_vector_t * vector2)
{
  if (!vector1 || !vector2)
    return -1;

  return _bit_vector_binary_op(BIT_VECTOR_XOR, vector1, vector2);
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
gds_bit_vector_t * bit_vector_create_from_string(char * cBitVector)
{
  gds_bit_vector_t * vector;
  uint32_t uLen;
  uint32_t uCptBits;
 
  uLen = strlen(cBitVector);
  vector = bit_vector_create(uLen);

  for (uCptBits = 0; uCptBits < uLen; uCptBits++) {
    if (cBitVector[uCptBits] != '0')
      bit_vector_set(vector, uCptBits);
  }
  return vector;
}

