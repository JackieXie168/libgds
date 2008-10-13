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

typedef int      (*FHashEltCompare) (const void * item1,
				     const void * item2,
				     unsigned int item_size);
typedef void     (*FHashEltDestroy) (void * item);
typedef uint32_t (*FHashCompute) (const void * item,
				  unsigned int hash_size);
typedef int      (*FHashForEach) (void * item, void * ctx);

typedef struct gds_hash_set_t gds_hash_set_t;

#define HASH_SUCCESS        0
#define HASH_SUCCESS_UNREF  1
#define HASH_ERROR_NO_MATCH -1

#ifdef __cplusplus
extern "C" {
#endif

  ///////////////////////////////////////////////////////////////////
  // HASH MAP
  ///////////////////////////////////////////////////////////////////


  ///////////////////////////////////////////////////////////////////
  // HASH SET
  ///////////////////////////////////////////////////////////////////

  // -----[ hash_create ]--------------------------------------------
  gds_hash_set_t * hash_set_create(unsigned int size,
				   float resize_threshold,
				   FHashEltCompare elt_cmp,
				   FHashEltDestroy elt_destroy,
				   FHashCompute hash_compute);
  // -----[ hash_set_add ]-------------------------------------------
  void * hash_set_add(gds_hash_set_t * hash, void * item);
  // -----[ hash_set_remove ]----------------------------------------
  int hash_set_remove(gds_hash_set_t * hash, void * item);
  // -----[ hash_set_search ]----------------------------------------
  void * hash_set_search(const gds_hash_set_t * hash, void * item);
  // -----[ hash_set_destroy ]---------------------------------------
  void hash_set_destroy(gds_hash_set_t ** hash_ref);

  // -----[ hash_set_get_refcnt ]------------------------------------
  unsigned int hash_set_get_refcnt(const gds_hash_set_t * hash,
				   void * item);
  // -----[ hash_set_for_each ]--------------------------------------
  int hash_set_for_each(const gds_hash_set_t * hash,
			FHashForEach foreach,
			void * ctx);
  // -----[ hash_set_for_each_key ]----------------------------------
  int hash_set_for_each_key(const gds_hash_set_t * hash,
			    FHashForEach foreach, 
			    void * ctx);
  // -----[ hash_set_get_enum ]--------------------------------------
  gds_enum_t * hash_set_get_enum(gds_hash_set_t * hash);
  // -----[ hash_set_dump ]------------------------------------------
  void hash_dump(const gds_hash_set_t * hash);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_HASH_H__ */
