// ==================================================================
// @(#)hash_utils.h
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 14/12/2004
// $Id$
// ==================================================================

/**
 * \file
 * Provide hash functions and item comparison functions.
 */

#ifndef __GDS_HASH_UTILS_H__
#define __GDS_HASH_UTILS_H__

#include <libgds/types.h>

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ hash_utils_key_compute_string ]--------------------------
  /**
   * Universal hash function for string keys (discussed in Sedgewick's
   * "Algorithms in C, 3rd edition") and slightly adapted.
   */
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
