// ==================================================================
// @(#)bloom_hash.h
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 15/03/2007
// $Id$
// ==================================================================

#ifndef __BLOOM_HASH_H__
#define __BLOOM_HASH_H__

#include <libgds/types.h>
#include <libgds/array.h>

typedef struct _BloomFilterHash SBloomFilterHash;

// ----- bloom_hash_create -------------------------------------------
SBloomFilterHash * bloom_hash_create(uint32_t uMaxValue, 
				      uint8_t uNbrHash);
// ----- bloom_hash_destroy ------------------------------------------
void bloom_hash_destroy(SBloomFilterHash ** pBloomHash);
// ----- bloom_hash_reset --------------------------------------------
void bloom_hash_reset(SBloomFilterHash * pBloomHash);
// ----- bloom_hash_get ----------------------------------------------
uint32_array_t * bloom_hash_get(SBloomFilterHash * pBloomHash, 
				uint8_t * pKey, 
				uint32_t uLen);

#endif /* __BLOOM_HASH_H__ */
