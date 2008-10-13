// ==================================================================
// @(#)radix-tree.h
//
// A library of function that handles radix-trees intended to store
// IPv4 prefixes.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 22/10/2002
// $Id$
// ==================================================================

#ifndef __GDS_RADIX_TREE_H__
#define __GDS_RADIX_TREE_H__

#include <libgds/enumerator.h>
#include <libgds/types.h>

// ----- pointer to free function for radix-tree items --------------
typedef void (*FRadixTreeDestroy)(void ** ppItem);
// ----- pointer to list function for radix-tree items --------------
typedef int (*FRadixTreeForEach)(uint32_t key, uint8_t key_len,
				 void * data, void * ctx);

// ----- structure of a radix-tree ----------------------------------
typedef struct gds_radix_tree_t {
  struct _radix_tree_item_t * root;
  uint8_t                     key_len;
  FRadixTreeDestroy           fDestroy;
} gds_radix_tree_t;

#ifdef __cplusplus
extern "C" {
#endif

  // ----- radix_tree_create ------------------------------------------
  gds_radix_tree_t * radix_tree_create(uint8_t key_len,
				       FRadixTreeDestroy fDestroy);
  // ----- radix_tree_destroy -----------------------------------------
  void radix_tree_destroy(gds_radix_tree_t ** tree_ref);
  // ----- radix_tree_add ---------------------------------------------
  int radix_tree_add(gds_radix_tree_t * tree, uint32_t key,
		     uint8_t key_len, void * data);
  // ----- radix_tree_remove ------------------------------------------
  int radix_tree_remove(gds_radix_tree_t * tree, uint32_t key,
			uint8_t key_len, int iSingle);
  // ----- radix_tree_get_exact ---------------------------------------
  void * radix_tree_get_exact(gds_radix_tree_t * tree,
			      uint32_t key,
			      uint8_t key_len);
  // ----- radix_tree_get_best ----------------------------------------
  void * radix_tree_get_best(gds_radix_tree_t * tree,
			     uint32_t key,
			     uint8_t key_len);
  // ----- radix_tree_for_each ----------------------------------------
  int radix_tree_for_each(gds_radix_tree_t * tree,
			  FRadixTreeForEach fForEach,
			  void * ctx);
  
  // ----- radix_tree_num_nodes ---------------------------------------
  int radix_tree_num_nodes(gds_radix_tree_t * tree, int with_data);
  // -----[ radix_tree_get_enum ]--------------------------------------
  gds_enum_t * radix_tree_get_enum(gds_radix_tree_t * tree);
  
#ifdef __cplusplus
}
#endif

#endif /* __GDS_RADIX_TREE_H__ */
