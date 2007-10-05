// ==================================================================
// @(#)bloom_hash.c
//
// @author Sebastien Tandel <sebastien [AT] tandel (dot) be>
// @date 15/03/2007
// @lastdate 15/03/2007
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libgds/memory.h>
#include <libgds/sha1.h>

#include <libgds/bloom_hash.h>

struct _BloomFilterHash {
  uint8_t uNbrHash;
  uint32_t uMaxValue;

  SSHA1Context pSHA1Ctx;
  /* TODO : if we want to have several digest functions (not only sha1 as now,
   * md5, others?)
   * we should put the length of the hash digest here too.
  FHashDigest fDigest;*/
};

#define SHA1_HASH_LENGTH 20

/**
 *
 */
SBloomFilterHash * bloom_hash_create(uint32_t uMaxValue, uint8_t uNbrHash)
{
  SBloomFilterHash * pBloomHash;

  if (uNbrHash > SHA1_HASH_LENGTH)
    return NULL;
 
  pBloomHash = MALLOC(sizeof(SBloomFilterHash));
  pBloomHash->uMaxValue = uMaxValue;
  pBloomHash->uNbrHash = uNbrHash;
  bloom_hash_reset(pBloomHash);

  return pBloomHash;
}

/**
 *
 */
void bloom_hash_destroy(SBloomFilterHash ** pBloomHash)
{
  if ( (*pBloomHash) ) {
    FREE( (*pBloomHash) );
    *pBloomHash = NULL;
  }
}

/**
 *
 */
void bloom_hash_reset(SBloomFilterHash * pBloomHash)
{
  if (!pBloomHash)
    return;

  sha1_starts(&pBloomHash->pSHA1Ctx);
}

/**
 *
 */
static uint32_t _byte_array_to_int(uint8_t uArray[], uint8_t uLen) 
{
  uint32_t val = 0;
  uint8_t uCpt;

  if (!uArray)
    return -1;

  for (uCpt = 0; uCpt < uLen; uCpt++) {
    val |= (uArray[uCpt] & 0xff) << ((uLen - 1 - uCpt) * 8);
  }
  return val;
}

/**
 *
 */
static SUInt32Array * _bloom_hash_get_int_array(SBloomFilterHash * pBloomHash, uint8_t shaSum[])
{
  SUInt32Array * uArray;
  uint32_t uNbrBytePerInt;
  uint32_t uOffset = 0;
  uint32_t uCpt;
  uint32_t result;

  if (!pBloomHash)
    return NULL;

  uArray = uint32_array_create(0);
  _array_set_length((SArray*)uArray, pBloomHash->uNbrHash);

  uNbrBytePerInt= SHA1_HASH_LENGTH / pBloomHash->uNbrHash;
  for (uCpt = 0; uCpt < pBloomHash->uNbrHash; uCpt++) {
    result = _byte_array_to_int(shaSum+uOffset, uNbrBytePerInt) % pBloomHash->uMaxValue;
    _array_set_at((SArray*)uArray, uCpt, &result);
    uOffset += uNbrBytePerInt;
  }
  return uArray;
}

/**
 *
 */
SUInt32Array * bloom_hash_get(SBloomFilterHash * pBloomHash, uint8_t * pKey, uint32_t uLen)
{
  uint8_t shaSum[SHA1_HASH_LENGTH];
  sha1_update(&pBloomHash->pSHA1Ctx, pKey, uLen);
  sha1_finish(&pBloomHash->pSHA1Ctx, shaSum);

  return _bloom_hash_get_int_array(pBloomHash, shaSum);
}

