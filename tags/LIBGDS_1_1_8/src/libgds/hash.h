// ==================================================================
// @(#)hash.h
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 03/12/2004
// @lastdate 03/12/2004
// ==================================================================

#ifndef __GDS_HASH_H__
#define __GDS_HASH_H__

#include <libgds/types.h>

typedef int (*FHashEltCompare) (void * pElt1, void * pElt2, uint32_t uEltSize);
typedef void (*FHashEltDestroy) (void * pElt);
//Do not have to return the key but only some value on which the implementation
//will do the modulo to obtain the real key. In fact, we can't return the real key
//because the hash table is dynamic. Thus, the maximum value of a key will grow
//as the size of the hash will grow. The initial value of the size of the hash 
//is just a value to begin with but will change with time.
typedef uint32_t (*FHashCompute) (void * pElt);

typedef void (*FHashForEach) (void * pElt, void * pContext);

typedef struct HashTable SHash;

// ----- hash_init ---------------------------------------------------
SHash * hash_init(const uint32_t uHashSize, float fResizeThreshold,
					    FHashEltCompare fEltCompare, 
					    FHashEltDestroy fEltDestroy, 
					    FHashCompute fHashCompute);
// ----- hash_add ----------------------------------------------------
int hash_add(SHash * pHash, void * pElt);
// ----- hash_del ----------------------------------------------------
int hash_del(SHash * pHash, void * pElt);
// ----- hash_search -------------------------------------------------
void * hash_search(SHash * pHash, void * pElt);
// ---- hash_destroy -------------------------------------------------
void hash_destroy(SHash ** pHash);

#endif //__GDS_HASH_H
