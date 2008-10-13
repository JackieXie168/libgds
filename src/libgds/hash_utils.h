// ==================================================================
// @(#)hash.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 14/12/2004
// $Id$
// ==================================================================

#ifndef __GDS_HASH_UTILS_H__
#define __GDS_HASH_UTILS_H__

#include <libgds/types.h>

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ hash_utils_key_compute_string ]--------------------------
  uint32_t hash_utils_key_compute_string(const void * item,
					 unsigned int hash_size);
  // -----[ hash_utils_compare_string ]------------------------------
  int hash_utils_compare_string(const void * item1,
				const void * item2,
				unsigned int item_size);
  
#ifdef __cplusplus
}
#endif

#endif /* __GDS_HASH_UTILS_H__ */
