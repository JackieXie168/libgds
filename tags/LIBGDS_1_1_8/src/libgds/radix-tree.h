// ==================================================================
// @(#)radix-tree.h
//
// A library of function that handles radix-trees intended to store
// IPv4 prefixes.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 22/10/2002
// @lastdate 21/05/2003
// ==================================================================

#ifndef __RADIX_TREE_H__
#define __RADIX_TREE_H__

#include <libgds/types.h>

// ----- pointer to free function for radix-tree items --------------
typedef void (*FRadixTreeDestroy)(void ** ppItem);
// ----- pointer to list function for radix-tree items --------------
typedef int (*FRadixTreeForEach)(uint32_t uKey, uint8_t uKeyLen,
				 void * pItem, void * pContext);

// ----- structure of a radix-tree node -----------------------------
typedef struct RadixTreeItem {
  struct RadixTreeItem * pLeft;  // 0
  struct RadixTreeItem * pRight; // 1
  void * pItem;
} SRadixTreeItem;

// ----- structure of a radix-tree ----------------------------------
typedef struct {
  SRadixTreeItem * pRoot;
  uint8_t uKeyLen;
  FRadixTreeDestroy fDestroy;
} SRadixTree;

// ----- radix_tree_create ------------------------------------------
extern SRadixTree * radix_tree_create(uint8_t uKeyLen,
				      FRadixTreeDestroy fDestroy);
// ----- radix_tree_destroy -----------------------------------------
extern void radix_tree_destroy(SRadixTree ** ppTree);
// ----- radix_tree_add ---------------------------------------------
extern int radix_tree_add(SRadixTree * pTree, uint32_t uKey,
			  uint8_t uKeyLen, void * pItem);
// ----- radix_tree_remove ------------------------------------------
extern int radix_tree_remove(SRadixTree * pTree, uint32_t uKey,
			     uint8_t uKeyLen, int iSingle);
// ----- radix_tree_get_exact ---------------------------------------
extern void * radix_tree_get_exact(SRadixTree * pTree,
				   uint32_t uKey,
				   uint8_t uKeyLen);
// ----- radix_tree_get_best ----------------------------------------
extern void * radix_tree_get_best(SRadixTree * pTree,
				  uint32_t uKey,
				  uint8_t uKeyLen);
// ----- radix_tree_for_each ----------------------------------------
extern int radix_tree_for_each(SRadixTree * pTree,
			       FRadixTreeForEach fForEach,
			       void * pContext);

#endif
