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
typedef uint32_t (*FHashCompute) (void * pElt);

typedef struct HashTable SHash;

// ----- hash_init ---------------------------------------------------
SHash * hash_init(const uint32_t uHashSize, FHashEltCompare fEltCompare, 
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
