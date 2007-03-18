// ==================================================================
// @(#)bloom_filter.h
//
// @author Sebastien Tandel <sebastien [AT] tandel (dot) be>
// @date 15/03/2007
// @lastdate 15/03/2007
// ==================================================================

#ifndef __BLOOM_FILTER_H__
#define __BLOOM_FILTER_H__

#include <libgds/types.h>

typedef struct _BloomFilter SBloomFilter;

SBloomFilter * bloom_filter_create(uint32_t uSize, uint32_t uNbrHashFn);
void bloom_filter_destroy(SBloomFilter ** pBloomFilter);
int8_t bloom_filter_add(SBloomFilter * pBloomFilter, uint8_t *uKey, uint32_t uKeyLen);
int8_t bloom_filter_add_array(SBloomFilter * pBloomFilter, uint8_t *uKey[]);
uint8_t bloom_filter_is_member(SBloomFilter * pBloomFilter, uint8_t * uKey, uint32_t uKeyLen);
char * bloom_filter_to_string(SBloomFilter * pBloomFilter);

#endif /* __BLOOM_FILTER_H__ */
