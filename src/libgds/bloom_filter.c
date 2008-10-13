// ==================================================================
// @(#)bloom_filter.c
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

#include <libgds/bloom_filter.h>
#include <libgds/bloom_hash.h>
#include <libgds/memory.h>
#include <libgds/array.h>
#include <libgds/bit_vector.h>
#include <string.h>

#define BLOOM_FILTER_SEGMENT_LEN 32

struct _BloomFilter {
  /* TODO STA : get rid of bloom_hash! */
  uint32_t           uSize;
  uint32_t           uNbrHashFn;

  gds_bit_vector_t * pBitVector;
  SBloomFilterHash * pBloomHash;
};

/**
 * @brief Creates a bloom filter
 *
 * @param uSize size of the bloom filter (size of the bit vector)
 * @param uNbrHash number of hash digest per key (with SHA1, 20 max.)
 *
 * @return a new allocated bloom filter.
 *
 * @warning It is the responsibility of the caller to free the returned bloom
 * filter with bloom_filter_destroy().
 */
SBloomFilter * bloom_filter_create(uint32_t uSize, uint32_t uNbrHash)
{
  SBloomFilter * pBloomFilter;
  SBloomFilterHash * pHash;

  pHash = bloom_hash_create(uSize, uNbrHash);
  if (!pHash)
    return NULL;

  pBloomFilter = MALLOC(sizeof(SBloomFilter));
  pBloomFilter->uSize = uSize;
  pBloomFilter->uNbrHashFn = uNbrHash;
  pBloomFilter->pBloomHash = pHash;
  pBloomFilter->pBitVector = bit_vector_create(uSize);

  return pBloomFilter;
}

/**
 * @brief Destroys a bloom filter
 *
 * @param ppBloomFilter a bloom filter
 */
void bloom_filter_destroy(SBloomFilter ** ppBloomFilter)
{
  if ( (*ppBloomFilter) ) {
    if ( (*ppBloomFilter)->pBitVector ) {
      bit_vector_destroy( &((*ppBloomFilter)->pBitVector) );
    }
    if ( (*ppBloomFilter)->pBloomHash ) {
      bloom_hash_destroy( &(*ppBloomFilter)->pBloomHash );
    }
    FREE( (*ppBloomFilter) );
    *ppBloomFilter = NULL;
  }
}

static int _bloom_filter_add_for_each(const void * pItem, const void * pCtx)
{
  uint32_t uItem = *(uint32_t *)pItem;
  SBloomFilter * pBloomFilter = (SBloomFilter *)pCtx;

//  printf("%d|", uItem);
  bit_vector_set(pBloomFilter->pBitVector, uItem);

  return 0;
}

static uint32_array_t * _bloom_filter_hash_get(SBloomFilter * pBloomFilter, uint8_t * uKey, uint32_t uKeyLen)
{
  if (!uKey || !pBloomFilter)
    return NULL;

  /* We should reset the hash function as the digest is dependent of the
   * previous input characters
   */
  bloom_hash_reset(pBloomFilter->pBloomHash);
  return bloom_hash_get(pBloomFilter->pBloomHash, uKey, uKeyLen);
}

/**
 * @brief Adds a key to a bloom filter
 *
 * @param pBloomFilter the bloom filter
 * @param uKey the key to add to the bloom filter. This key is an array of
 * byte. It is not mandatory to have a NULL character at the end of the array.
 * @param uKeyLen the length of the key
 *
 * @return 0 if the key is added to the bloom filter. If the key or the bloom
 * filter is NULL, -1 is returned.
 */
int8_t bloom_filter_add(SBloomFilter * pBloomFilter, uint8_t *uKey, uint32_t uKeyLen)
{
  uint32_array_t * puArray;

  if (!uKey || !pBloomFilter)
    return -1;

  puArray = _bloom_filter_hash_get(pBloomFilter, uKey, uKeyLen);
//  printf("bloom filter> add %s:|", uKey);
  uint32_array_for_each(puArray, _bloom_filter_add_for_each, pBloomFilter);
// printf("\n");
  uint32_array_destroy( &puArray );
  return 0;
}

/**
 * @brief Adds an array of byte array to a bloom filter
 *
 * @param pBloomFilter
 * @param key[] array of byte representing the keys. Each element must be
 * NULL-terminated because the function is using strlen() to determine the
 * length.
 *
 * @return 0 if the array of keys is added to the bloom filter. If the key or
 * the bloom filter is NULL, -1 is returned.
 */
int8_t bloom_filter_add_array(SBloomFilter * pBloomFilter, uint8_t **uKey)
{
  uint32_t uCpt = 0;

  if (!uKey || !pBloomFilter)
    return -1;
  
  while (uKey[uCpt]) {
    bloom_filter_add(pBloomFilter, uKey[uCpt], strlen((char *)uKey[uCpt]));
    uCpt++;
  }
  return 0;
}

/**
 * @brief Creates a string representation of the bloom filter
 *
 * @param pBloomFilter the bloom filter
 *
 * @returns the string representation of the bloom filter or NULL if the bloom
 * filter is NULL.
 *
 * \warning It is the responsibility of the caller to free the returned string
 * (with FREE not free!).
 */
char * bloom_filter_to_string(SBloomFilter * pBloomFilter)
{
  if (!pBloomFilter || !pBloomFilter->pBitVector)
    return NULL;

  return bit_vector_to_string(pBloomFilter->pBitVector);
}

static int _bloom_filter_is_member_for_each(const void * pItem,
					    const void * pCtx)
{
  uint32_t uItem = *(uint32_t*) pItem;
  SBloomFilter * pBloomFilter = (SBloomFilter*)pCtx;

  //printf("%d|", uItem);
  return !(bit_vector_get(pBloomFilter->pBitVector, uItem) == 1);
}

/**
 * @brief Tests the membership of a key in a bloom filter.
 *
 * @param pBloomFilter the bloom filter
 * @param uKey the key to search in the bloom filter
 * @param uKeyLen the length of the key
 *
 * @return 1 if the key belongs to the bloom filter, else 0.
 */
uint8_t bloom_filter_is_member(SBloomFilter * pBloomFilter, uint8_t * uKey, uint32_t uKeyLen)
{
  uint32_array_t * puArray;
  int iRet;

  if (!uKey || !pBloomFilter)
    return 0;

  puArray = _bloom_filter_hash_get(pBloomFilter, uKey, uKeyLen);
 // printf("bloom filter> search %s:|", (char*)uKey);
  iRet = uint32_array_for_each(puArray, _bloom_filter_is_member_for_each, pBloomFilter);
 // printf("\n");
  uint32_array_destroy( &puArray );

  return (iRet == 0);
}

/**
 * @brief Perform a \em and operation on a bloom filter
 *
 *  The operation is performed if :
 *  - bloom filters are not NULL,
 *  - they have the same size,
 *  - they have the same number of digests msg per key.
 *
 * @param pBloomFilter1 the first operand of the \em and operation. This bloom
 * filter contains the result of the \em and operation.
 * @param pBloomFilter2 the second operand of the \em and operation.
 *
 * @return 0 if pBloomFilter1 has been \em anded, else -1 is returned.
 */
int8_t bloom_filter_and(SBloomFilter * pBloomFilter1, SBloomFilter * pBloomFilter2)
{
  if ( !pBloomFilter1 || !pBloomFilter2 )
    return -1;

  if (pBloomFilter1->uSize != pBloomFilter2->uSize)
    return -1;

  if (pBloomFilter1->uNbrHashFn != pBloomFilter2->uNbrHashFn)
    return -1;

  return bit_vector_and(pBloomFilter1->pBitVector, pBloomFilter2->pBitVector);
}

/**
 * @brief Perform a \em or operation on a bloom filter
 *
 * The operation is performed if :
 *  - bloom filters are not NULL,
 *  - they have the same size,
 *  - they have the same number of digests msg per key.
 *
 * @param pBloomFilter1 the first operand of the \em or operation. This bloom
 * filter contains the result of the \em or operation.
 * @param pBloomFilter2 the second operand of the \em or peration.
 *
 * @return 0 if pBloomFilter1 has been \em ored, else -1 is returned.
 */
int8_t bloom_filter_or(SBloomFilter * pBloomFilter1, SBloomFilter * pBloomFilter2)
{
  if ( !pBloomFilter1 || !pBloomFilter2 )
    return -1;

  if (pBloomFilter1->uSize != pBloomFilter2->uSize)
    return -1;

  if (pBloomFilter1->uNbrHashFn != pBloomFilter2->uNbrHashFn)
    return -1;

  return bit_vector_or(pBloomFilter1->pBitVector, pBloomFilter2->pBitVector);
}

/**
 * @brief Perform a \em xor operation on a bloom filter
 *
 *  The operation is performed if :
 *  - bloom filters are not NULL,
 *  - they have the same size,
 *  - they have the same number of digests msg per key.
 *
 * @param pBloomFilter1 the first operand of the \em xor operation. This bloom
 * filter contains the result of the \em xor operation.
 * @param pBloomFilter2 the second operand of the \em xor peration.
 *
 * @return 0 if pBloomFilter1 has been \em xored, else -1 is returned.
 */
int8_t bloom_filter_xor(SBloomFilter * pBloomFilter1, SBloomFilter * pBloomFilter2)
{
  if ( !pBloomFilter1 || !pBloomFilter2 )
    return -1;

  if (pBloomFilter1->uSize != pBloomFilter2->uSize)
    return -1;

  if (pBloomFilter1->uNbrHashFn != pBloomFilter2->uNbrHashFn)
    return -1;

  return bit_vector_xor(pBloomFilter1->pBitVector, pBloomFilter2->pBitVector);
}

/**
 * @brief Tests the equality of two bloom filters
 *
 * @param pBloomFilter1 first bloom filter
 * @param pBloomFilter2 second bloom filter
 *
 * @return 0 if both bloom filters have the same number of digest msg per key
 * and the same bit vector. 
 *
 * @see bit_vector_equals()
 */
int8_t bloom_filter_equals(SBloomFilter * pBloomFilter1, SBloomFilter * pBloomFilter2)
{
  if (!pBloomFilter1 && pBloomFilter2)
    return 1;

  if (!pBloomFilter1 || !pBloomFilter2)
    return 0;

  if (pBloomFilter1->uNbrHashFn != pBloomFilter2->uNbrHashFn)
    return 0;

  return bit_vector_equals(pBloomFilter1->pBitVector, pBloomFilter2->pBitVector);
}

