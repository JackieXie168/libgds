// ==================================================================
// @(#)radix-tree.c
//
// A library of function that handles radix-trees intended to store
// IPv4 prefixes.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 21/10/2002
// @lastdate 08/11/2005
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <libgds/memory.h>
#include <libgds/radix-tree.h>
#include <libgds/stack.h>

// ----- radix_tree_item_create -------------------------------------
/**
 *
 */
SRadixTreeItem * radix_tree_item_create(void * pItem)
{
  SRadixTreeItem * pTreeItem=
    (SRadixTreeItem *) MALLOC(sizeof(SRadixTreeItem));
  pTreeItem->pLeft= NULL;
  pTreeItem->pRight= NULL;
  pTreeItem->pItem= pItem;
  return pTreeItem;
}

// ----- radix_tree_item_destroy ------------------------------------
/**
 * Remove an item. Remove also all its children if the parameter
 * 'iSingle' is 1.
 */
void radix_tree_item_destroy(SRadixTreeItem ** ppTreeItem,
			     FRadixTreeDestroy fDestroy,
			     int iSingle)
{
  SStack * pStack= stack_create(32);
  SRadixTreeItem * pTreeItem= *ppTreeItem;

  while (pTreeItem != NULL) {

    /* If all children are to be removed, push them onto stack. */
    if (!iSingle) {
      if (pTreeItem->pLeft != NULL)
	stack_push(pStack, pTreeItem->pLeft);
      if (pTreeItem->pRight != NULL)
	stack_push(pStack, pTreeItem->pRight);
    }

    /* Destroy the item. */
    if ((pTreeItem->pItem != NULL) && (fDestroy != NULL)) {
      fDestroy(&pTreeItem->pItem);
      pTreeItem->pItem= NULL;
    }

    /* If the current item is empty (no child) or if we delete all
       child, then free the item's memory. */
    if (((pTreeItem->pLeft == NULL) && (pTreeItem->pRight == NULL)) ||
	!iSingle) {
      FREE(pTreeItem);
      *ppTreeItem= NULL;
    }

    /* Any other child to be removed ? */
    if (stack_depth(pStack) > 0)
      pTreeItem= (SRadixTreeItem *) stack_pop(pStack);
    else
      pTreeItem= NULL;

  }
  stack_destroy(&pStack);
}

// ----- radix_tree_create ------------------------------------------
/**
 *
 */
SRadixTree * radix_tree_create(uint8_t uKeyLen,
			       FRadixTreeDestroy fDestroy)
{
  SRadixTree * pTree= (SRadixTree *) MALLOC(sizeof(SRadixTree));
  pTree->pRoot= NULL;
  pTree->uKeyLen= uKeyLen;
  pTree->fDestroy= fDestroy;
  return pTree;
}

// ----- radix_tree_destroy -----------------------------------------
/**
 * Free the whole radix-tree.
 */
void radix_tree_destroy(SRadixTree ** ppTree)
{
  if (*ppTree != NULL) {
    if ((*ppTree)->pRoot != NULL)
      radix_tree_item_destroy(&(*ppTree)->pRoot, (*ppTree)->fDestroy, 0);
    FREE(*ppTree);
    *ppTree= NULL;
  }
}

// ----- radix_tree_add ---------------------------------------------
/**
 * Add an 'item' in the radix-tree at 'key/len' position.
 */
int radix_tree_add(SRadixTree * pTree, uint32_t uKey,
		   uint8_t uKeyLen, void * pItem)
{
  SRadixTreeItem ** ppTreeItem= &pTree->pRoot;
  uint8_t uLen= uKeyLen;

  // Go to given 'key/len' position. Create path along the way if
  // required...
  // Warning: '*ppTreeItem' is used to add new nodes while
  // 'ppTreeItem' is used to keep track of the current node !!
  while (uLen > 0) {
    if (*ppTreeItem == NULL)
      *ppTreeItem= radix_tree_item_create(NULL);
    if (uKey & (1 << (pTree->uKeyLen-(uKeyLen+1-uLen))))
      ppTreeItem= &(*ppTreeItem)->pRight;
    else
      ppTreeItem= &(*ppTreeItem)->pLeft;
    uLen--;
  }

  if (*ppTreeItem == NULL) {
    *ppTreeItem= radix_tree_item_create(pItem);
  } else {
    // If a previous value exists, replace it
    if ((*ppTreeItem)->pItem != NULL) {
      if (pTree->fDestroy != NULL)
	pTree->fDestroy(&(*ppTreeItem)->pItem);
    }
    // Set new value
    (*ppTreeItem)->pItem= pItem;
  }

  return 0;
}

// ----- radix_tree_remove ------------------------------------------
/**
 * Remove the item at position 'key/Len' as well as all the empty
 * nodes that are on the way.
 *
 * Parameters:
 * - iSingle, if 1 remove a single key otherwise, remove the key and
 *   all the keys under.
 */
int radix_tree_remove(SRadixTree * pTree, uint32_t uKey,
		      uint8_t uKeyLen, int iSingle)
{
  SStack * pStack= stack_create(pTree->uKeyLen);
  uint8_t uLen= uKeyLen;
  SRadixTreeItem ** ppTreeItem= &pTree->pRoot;
  int iEmpty;
  
  while (uLen > 0) {
    if (*ppTreeItem == NULL)
      return -1;
    if (uKey & (1 << (pTree->uKeyLen-(uKeyLen+1-uLen)))) {
      if ((*ppTreeItem)->pRight != NULL) {
	stack_push(pStack, ppTreeItem);
	ppTreeItem= &(*ppTreeItem)->pRight;
      } else
	return -1;
    } else {
      if ((*ppTreeItem)->pLeft != NULL) {
	stack_push(pStack, ppTreeItem);
	ppTreeItem= &(*ppTreeItem)->pLeft;
      } else
	return -1;
    }
    uLen--;
  }
  if ((*ppTreeItem == NULL) || ((*ppTreeItem)->pItem == NULL))
    return -1;

  /* Keep information on the current key's emptiness. The key is
     considered empty if it has no child and has no item. */
  iEmpty= (((*ppTreeItem)->pLeft == NULL)
	   && ((*ppTreeItem)->pRight == NULL));

  radix_tree_item_destroy(ppTreeItem, pTree->fDestroy, iSingle);

  /* If the current item is empty (no key below, go up towards the
     radix-tree's root and clear keys until a non-empty is found. */
  while (iEmpty && (stack_depth(pStack) > 0)) {
    ppTreeItem= (SRadixTreeItem **) stack_pop(pStack);

    /* If the key is empty (no child and no item), remove it. */
    if (((*ppTreeItem)->pLeft == NULL) &&
	((*ppTreeItem)->pRight == NULL) &&
	((*ppTreeItem)->pItem == NULL)) {
      radix_tree_item_destroy(ppTreeItem, pTree->fDestroy, 1);
    } else
      break;
  }
  stack_destroy(&pStack);
  return 0;
}

// ----- radix_tree_get_exact ---------------------------------------
/**
 * Return the item exactly at position 'key/len'.
 */
void * radix_tree_get_exact(SRadixTree * pTree,
			    uint32_t uKey,
			    uint8_t uKeyLen)
{
  uint8_t uLen= uKeyLen;
  SRadixTreeItem * pTreeItem= pTree->pRoot;

  while (uLen > 0) {
    if (pTreeItem == NULL)
      return NULL;
    if (uKey & (1 << (pTree->uKeyLen-(uKeyLen+1-uLen)))) {
      if (pTreeItem->pRight != NULL)
	pTreeItem= pTreeItem->pRight;
      else
	return NULL;
    } else {
      if (pTreeItem->pLeft != NULL)
	pTreeItem= pTreeItem->pLeft;
      else
	return NULL;
    }
    uLen--;
  }
  if (pTreeItem != NULL)
    return pTreeItem->pItem;
  return NULL;
}

// ----- radix_tree_get_best ----------------------------------------
/**
 * Return the item that best matches position 'key/len'.
 */
void * radix_tree_get_best(SRadixTree * pTree,
			   uint32_t uKey,
			   uint8_t uKeyLen)
{
  uint8_t uLen= uKeyLen;
  SRadixTreeItem * pTreeItem= pTree->pRoot;
  void * pResult= NULL;

  /* If the tree is empty, there is no possible match. */
  if (pTreeItem == NULL)
    return NULL;

  /* Otherwize, the shortest match corresponds to the root. */
  if (pTreeItem->pItem != NULL)
    pResult= pTreeItem->pItem;

  /* Go down the tree, as long as the requested key matches the
     traversed prefixes and as deep as the requested key length... */
  while (uLen > 0) {

    if (uKey & (1 << (pTree->uKeyLen-(uKeyLen+1-uLen)))) {
      // Bit is 1
      if (pTreeItem->pRight != NULL)
	pTreeItem= pTreeItem->pRight;
      else
	break;
    } else {
      // Bit is 0
      if (pTreeItem->pLeft != NULL)
	pTreeItem= pTreeItem->pLeft;
      else
	break;
    }
    uLen--;

    if ((pTreeItem != NULL) && (pTreeItem->pItem != NULL))
      pResult= pTreeItem->pItem;
  }
  
  return pResult;
}

typedef struct {
  SRadixTreeItem * pTreeItem;
  uint32_t uKey;
  uint8_t uKeyLen;
} SRadixTreeForEachContext;

// ----- radix_tree_for_each ----------------------------------------
/**
 * Call the 'fForEach' function for each non empty node.
 */
int radix_tree_for_each(SRadixTree * pTree,
			FRadixTreeForEach fForEach,
			void * pContext)
{
  SStack * pStack= stack_create(pTree->uKeyLen);
  SRadixTreeItem * pTreeItem;
  SRadixTreeForEachContext * pStackContext;
  int iResult= 0;
  uint32_t uKey;
  uint8_t uKeyLen;

  pTreeItem= pTree->pRoot;
  uKey= 0;
  uKeyLen= 0;
  // Depth first search
  while (pTreeItem != NULL) {
    if (pTreeItem->pItem != NULL) {
      iResult= fForEach(uKey, uKeyLen, pTreeItem->pItem, pContext);
      if (iResult != 0)
	return iResult;
    }
    if (pTreeItem->pLeft != NULL) {
      if (pTreeItem->pRight != NULL) {
	pStackContext= (SRadixTreeForEachContext *)
	  MALLOC(sizeof(SRadixTreeForEachContext));
	pStackContext->pTreeItem= pTreeItem->pRight;
	pStackContext->uKeyLen= uKeyLen+1;
	pStackContext->uKey= uKey+(1 << (pTree->uKeyLen-uKeyLen-1));
	stack_push(pStack, pStackContext);
      }
      pTreeItem= pTreeItem->pLeft;
      uKeyLen++;
    } else if (pTreeItem->pRight != NULL) {
      pTreeItem= pTreeItem->pRight;
      uKey= uKey+(1 << (pTree->uKeyLen-uKeyLen-1));
      uKeyLen++;
    } else {
      if (stack_depth(pStack) > 0) {
	pStackContext= (SRadixTreeForEachContext *) stack_pop(pStack);
	pTreeItem= pStackContext->pTreeItem;
	uKey= pStackContext->uKey;
	uKeyLen= pStackContext->uKeyLen;
	FREE(pStackContext);
      } else
	break;
    }
  }
  stack_destroy(&pStack);
  return 0;
}

// ----- _radix_tree_item_num_nodes ---------------------------------
int _radix_tree_item_num_nodes(SRadixTreeItem * pTreeItem)
{
  if (pTreeItem != NULL) {
    return 1 + _radix_tree_item_num_nodes(pTreeItem->pLeft)+
      _radix_tree_item_num_nodes(pTreeItem->pRight);
  }
  return 0;
}

// ----- radix_tree_num_nodes ---------------------------------------
/**
 * Count the number of nodes in the tree. The algorithm uses a
 * divide-and-conquer recursive approach.
 */
int radix_tree_num_nodes(SRadixTree * pTree)
{
  return _radix_tree_item_num_nodes(pTree->pRoot);
}
