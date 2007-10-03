// ==================================================================
// @(#)hash.h
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 03/12/2004
// @lastdate 12/12/2005
// ==================================================================

#ifndef __GDS_HASH_H__
#define __GDS_HASH_H__

#include <libgds/enumerator.h>
#include <libgds/types.h>

typedef int (*FHashEltCompare) (void * pElt1, void * pElt2,
                                unsigned int uEltSize);
typedef void (*FHashEltDestroy) (void * pElt);
//Do not have to return the key but only some value on which the implementation
//will do the modulo to obtain the real key. In fact, we can't return the real key
//because the hash table is dynamic. Thus, the maximum value of a key will grow
//as the size of the hash will grow. The initial value of the size of the hash 
//is just a value to begin with but will change with time.
typedef uint32_t (*FHashCompute) (const void * pElt, const uint32_t uHashSize);

typedef int (*FHashForEach) (void * pElt, void * pContext);

typedef struct HashTable SHash;

// ----- hash_init ---------------------------------------------------
extern SHash * hash_init(const uint32_t uHashSize,
			 const float fResizeThreshold,
			 FHashEltCompare fEltCompare, 
			 FHashEltDestroy fEltDestroy, 
			 FHashCompute fHashCompute);
// ----- hash_add ----------------------------------------------------
extern int hash_add(SHash * pHash, void * pElt);
// ----- hash_del ----------------------------------------------------
extern int hash_del(SHash * pHash, void * pElt);
// ----- hash_search -------------------------------------------------
extern void * hash_search(const SHash * pHash, void * pElt);
// ---- hash_destroy -------------------------------------------------
extern void hash_destroy(SHash ** pHash);

// -----[ hash_info ]-------------------------------------------------
extern uint32_t hash_info(const SHash * pHash, void * pItem);
// -----[ hash_for_each ]---------------------------------------------
extern int hash_for_each(const SHash * pHash, FHashForEach fHashForEach, 
			 void * pContext);
// -----[ hash_for_each_key ]----------------------------------------
extern int hash_for_each_key(const SHash * pHash, FHashForEach fHashForEach, 
			     void * pContext);
// -----[ hash_get_enum ]--------------------------------------------
extern SEnumerator * hash_get_enum(SHash * pHash);

#endif //__GDS_HASH_H__