
// ==================================================================
// @(#)hash.h
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 03/12/2004
// $Id$
// ==================================================================

#ifndef __GDS_HASH_H__
#define __GDS_HASH_H__

#include <libgds/enumerator.h>
#include <libgds/types.h>

typedef int      (*FHashEltCompare) (void * item1, void * item2,
				     unsigned int item_size);
typedef void     (*FHashEltDestroy) (void * item);
typedef uint32_t (*FHashCompute) (const void * item,
				  const unsigned int hash_size);
typedef int      (*FHashForEach) (void * item, void * ctx);

typedef struct hash_table_t hash_t;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ hash_init ]----------------------------------------------
  hash_t * hash_init(const unsigned int size,
		     const float resize_threshold,
		     FHashEltCompare elt_cmp,
		     FHashEltDestroy elt_destroy,
		     FHashCompute hash_compute);
  // -----[ hash_add ]-----------------------------------------------
  void * hash_add(hash_t * hash, void * item);
  // -----[ hash_del ]-----------------------------------------------
  int hash_del(hash_t * hash, void * item);
  // -----[ hash_search ]--------------------------------------------
  void * hash_search(const hash_t * hash, void * item);
  // ---- hash_destroy ----------------------------------------------
  void hash_destroy(hash_t ** hash_ref);

  // -----[ hash_get_refcnt ]----------------------------------------
  unsigned int hash_get_refcnt(const hash_t * hash, void * item);
  // -----[ hash_for_each ]------------------------------------------
  int hash_for_each(const hash_t * hash, FHashForEach for_each,
		    void * ctx);
  // -----[ hash_for_each_key ]--------------------------------------
  int hash_for_each_key(const hash_t * hash, FHashForEach for_each, 
			void * ctx);
  // -----[ hash_get_enum ]------------------------------------------
  enum_t * hash_get_enum(hash_t * hash);

  // -----[ hash_dump ]----------------------------------------------
  void hash_dump(const hash_t * hash);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_HASH_H__ */
