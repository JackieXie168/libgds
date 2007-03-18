// ==================================================================
// @(#)bloom_filter.c
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

#include <libgds/bloom_filter.h>
#include <libgds/bloom_hash.h>
#include <libgds/memory.h>
#include <libgds/array.h>
#include <libgds/bit_vector.h>
#include <string.h>

#define BLOOM_FILTER_SEGMENT_LEN 32

struct _BloomFilter {
  /* TODO STA : get rid of bloom_hash! */
  uint32_t uSize;
  uint32_t uNbrHashFn;

  SBitVector * pBitVector;
  SBloomFilterHash * pBloomHash;
};

SBloomFilter * bloom_filter_create(uint32_t uSize, uint32_t uNbrHashFn)
{
  SBloomFilter * pBloomFilter;
  SBloomFilterHash * pHash;

  pHash = bloom_hash_create(uSize, uNbrHashFn);
  if (!pHash)
    return NULL;

  pBloomFilter = MALLOC(sizeof(SBloomFilter));
  pBloomFilter->uSize = uSize;
  pBloomFilter->uNbrHashFn = uNbrHashFn;
  pBloomFilter->pBloomHash = pHash;
  pBloomFilter->pBitVector = bit_vector_create(uSize);

  return pBloomFilter;
}

void bloom_filter_destroy(SBloomFilter ** pBloomFilter)
{
  if ( (*pBloomFilter) ) {
    if ( (*pBloomFilter)->pBitVector ) {
      bit_vector_destroy( &((*pBloomFilter)->pBitVector) );
    }
    if ( (*pBloomFilter)->pBloomHash ) {
      bloom_hash_destroy( &(*pBloomFilter)->pBloomHash );
    }
    FREE( (*pBloomFilter) );
    *pBloomFilter = NULL;
  }
}

static int _bloom_filter_add_for_each(void * pItem, void * pCtx)
{
  uint32_t uItem = *(uint32_t *)pItem;
  SBloomFilter * pBloomFilter = (SBloomFilter *)pCtx;

//  printf("%d|", uItem);
  bit_vector_set(pBloomFilter->pBitVector, uItem);

  return 0;
}

static SUInt32Array * _bloom_filter_hash_get(SBloomFilter * pBloomFilter, uint8_t * uKey, uint32_t uKeyLen)
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
 *
 */
void bloom_filter_add(SBloomFilter * pBloomFilter, uint8_t *uKey, uint32_t uKeyLen)
{
  SUInt32Array * puArray;

  if (!uKey || !pBloomFilter)
    return;

  puArray = _bloom_filter_hash_get(pBloomFilter, uKey, uKeyLen);
//  printf("bloom filter> add %s:|", uKey);
  _array_for_each( (SArray *)puArray, _bloom_filter_add_for_each, pBloomFilter);
// printf("\n");
  uint32_array_destroy( &puArray );
}

/**
 * @param pBloomFilter
 * @param key[] array of char* terminated by NULL. should be able to determine
 * the length of each elt with strlen()
 */
void bloom_filter_add_array(SBloomFilter * pBloomFilter, uint8_t *uKey[])
{
  uint32_t uCpt = 0;

  if (!uKey || !pBloomFilter)
    return;
  
  while (uKey[uCpt]) {
    bloom_filter_add(pBloomFilter, uKey[uCpt], strlen((char *)uKey[uCpt]));
    uCpt++;
  }
}


char * bloom_filter_to_string(SBloomFilter * pBloomFilter)
{
  if (!pBloomFilter || !pBloomFilter->pBitVector)
    return NULL;

  return bit_vector_to_string(pBloomFilter->pBitVector);
}

static int _bloom_filter_is_member_for_each(void * pItem, void * pCtx)
{
  uint32_t uItem = *(uint32_t*) pItem;
  SBloomFilter * pBloomFilter = (SBloomFilter*)pCtx;

  //printf("%d|", uItem);
  return !(bit_vector_get(pBloomFilter->pBitVector, uItem) == 1);
}


uint8_t bloom_filter_is_member(SBloomFilter * pBloomFilter, uint8_t * uKey, uint32_t uKeyLen)
{
  SUInt32Array * puArray;
  int iRet;

  if (!uKey || !pBloomFilter)
    return 0;

  puArray = _bloom_filter_hash_get(pBloomFilter, uKey, uKeyLen);
 // printf("bloom filter> search %s:|", (char*)uKey);
  iRet = _array_for_each( (SArray *)puArray, _bloom_filter_is_member_for_each, pBloomFilter);
 // printf("\n");
  uint32_array_destroy( &puArray );

  return (iRet == 0);
}
