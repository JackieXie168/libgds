// ==================================================================
// @(#)hash.h
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 03/12/2004
// @lastdate 18/07/2007
// ==================================================================

#ifndef __GDS_HASH_H__
#define __GDS_HASH_H__

#include <libgds/enumerator.h>
#include <libgds/types.h>

typedef int (*FHashEltCompare) (void * pElt1, void * pElt2,
                                unsigned int uEltSize);
typedef void (*FHashEltDestroy) (void * pElt);
typedef uint32_t (*FHashCompute) (const void * pElt,
				  const uint32_t uHashSize);
typedef int (*FHashForEach) (void * pElt, void * pContext);

typedef struct HashTable SHash;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ hash_init ]----------------------------------------------
  SHash * hash_init(const uint32_t uHashSize,
		    const float fResizeThreshold,
		    FHashEltCompare fEltCompare, 
		    FHashEltDestroy fEltDestroy, 
		    FHashCompute fHashCompute);
  // -----[ hash_add ]-----------------------------------------------
  void * hash_add(SHash * pHash, void * pElt);
  // -----[ hash_del ]-----------------------------------------------
  int hash_del(SHash * pHash, void * pElt);
  // -----[ hash_search ]--------------------------------------------
  void * hash_search(const SHash * pHash, void * pElt);
  // ---- hash_destroy ----------------------------------------------
  void hash_destroy(SHash ** pHash);

  // -----[ hash_get_refcnt ]----------------------------------------
  uint32_t hash_get_refcnt(const SHash * pHash, void * pItem);
  // -----[ hash_for_each ]------------------------------------------
  int hash_for_each(const SHash * pHash, FHashForEach fHashForEach, 
		    void * pContext);
  // -----[ hash_for_each_key ]--------------------------------------
  int hash_for_each_key(const SHash * pHash, FHashForEach fHashForEach, 
			void * pContext);
  // -----[ hash_get_enum ]------------------------------------------
  SEnumerator * hash_get_enum(SHash * pHash);

  // -----[ hash_dump ]----------------------------------------------
  void hash_dump(const SHash * pHash);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_HASH_H__ */
