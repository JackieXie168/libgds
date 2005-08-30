// ==================================================================
// @(#)patricia-tree.c
//
// Patricia tree implementation.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 17/05/2005
// @lastdate 10/08/2005
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <libgds/array.h>
#include <libgds/memory.h>
#include <libgds/patricia-tree.h>

// -----[ precomputed masks ]----------------------------------------
static trie_key_t trie_predef_masks[TRIE_KEY_SIZE+1];

// -----[ _trie_init ]-----------------------------------------------
/**
 * This function initializes the array of predifined masks. This makes
 * possible faster key masking.
 */
static void _trie_init()
{
  trie_key_len_t uIndex;

  trie_predef_masks[0]= 0;
  for (uIndex= 1; uIndex < TRIE_KEY_SIZE+1; uIndex++)
    trie_predef_masks[uIndex]= (trie_predef_masks[uIndex-1] |
				(1 << (TRIE_KEY_SIZE-uIndex)));
}

// -----[ trie_create ]----------------------------------------------
/**
 * Create a new Patricia tree.
 */
STrie * trie_create(FTrieDestroy fDestroy)
{
  STrie * pTrie= (STrie *) MALLOC(sizeof(STrie));
  pTrie->pRoot= NULL;
  pTrie->fDestroy= fDestroy;
  return pTrie;
}

// -----[ trie_item_create ]-----------------------------------------
/**
 * Create a new node for the Patricia tree. Note: the function will
 * take care of correctly masking the node's key according to its
 * length.
 */
static STrieItem * trie_item_create(trie_key_t uKey,
				    trie_key_len_t uKeyLen,
				    void * pData)
{
  STrieItem * pTrieItem= (STrieItem *) MALLOC(sizeof(STrieItem));
  pTrieItem->pLeft= NULL;
  pTrieItem->pRight= NULL;
  pTrieItem->uKey= uKey & trie_predef_masks[uKeyLen];
  pTrieItem->uKeyLen= uKeyLen;
  pTrieItem->pData= pData;
  return pTrieItem;
}

// -----[ longest_common_prefix ]------------------------------------
/**
 * Compute the longest common prefix between two given keys.
 *
 * Pre: (key lenghts <= TRIE_KEY_SIZE) &
 *      (ptKey and ptKeyLen are valid pointers)
 */
static void longest_common_prefix(trie_key_t tKey1,
				  trie_key_len_t tKeyLen1,
				  trie_key_t tKey2,
				  trie_key_len_t tKeyLen2,
				  trie_key_t * ptKey,
				  trie_key_len_t * ptKeyLen)
{
  trie_key_t tMask= 1 << (TRIE_KEY_SIZE-1);
  trie_key_len_t tMaxLen= ((tKeyLen1 <= tKeyLen2)?
			   tKeyLen1:tKeyLen2);
  *ptKey= 0;
  *ptKeyLen= 0;
  while (*ptKeyLen < tMaxLen) {
    if ((tKey1 & tMask) != (tKey2 & tMask))
      return;
    *ptKey|= (tKey1 & tMask);
    tMask= (tMask >> 1);
    (*ptKeyLen)++;
  }
}

// -----[ trie_item_insert ]-----------------------------------------
/**
 * Insert a new (key, value) pair into the Patricia tree. This
 * function is only an helper function. The 'trie_insert' function
 * should be used instead.
 *
 * Pre: (uKey length <= TRIE_KEY_SIZE)
 *
 * Result: 0 on success and -1 on error (duplicate key)
 */
static int trie_item_insert(STrieItem ** ppItem, trie_key_t uKey,
			    trie_key_len_t uKeyLen, void * pData,
			    FTrieDestroy fDestroy)
{
  trie_key_t tPrefix;
  trie_key_len_t tPrefixLen;
  STrieItem * pNewItem;

  // Find the longest common prefix
  longest_common_prefix((*ppItem)->uKey, (*ppItem)->uKeyLen,
			uKey, uKeyLen, &tPrefix, &tPrefixLen);

  // Split, append or recurse ?
  if ((tPrefixLen == uKeyLen) && (tPrefixLen == (*ppItem)->uKeyLen)) {
    if ((fDestroy != NULL) && ((*ppItem)->pData != NULL))
      fDestroy(&(*ppItem)->pData);
    (*ppItem)->pData= pData;
    return 0;
  } else if (tPrefixLen < (*ppItem)->uKeyLen) {
    // Split is required
    pNewItem= trie_item_create(tPrefix, tPrefixLen, NULL);
    if ((*ppItem)->uKey & (1 << (TRIE_KEY_SIZE-tPrefixLen-1))) {
      pNewItem->pRight= *ppItem;
    } else {
      pNewItem->pLeft= *ppItem;
    }
    if (tPrefixLen == uKeyLen) {
      pNewItem->pData= pData;
    } else {
      if (uKey & (1 << (TRIE_KEY_SIZE-tPrefixLen-1))) {
	pNewItem->pRight= trie_item_create(uKey, uKeyLen, pData);
      } else {
	pNewItem->pLeft= trie_item_create(uKey, uKeyLen, pData);
      }
    }
    *ppItem= pNewItem;
    return 0;
  } else {
    if (uKey & (1 << (TRIE_KEY_SIZE-(*ppItem)->uKeyLen-1))) {
      if ((*ppItem)->pRight != NULL) {
	// Recurse
	return trie_item_insert(&(*ppItem)->pRight, uKey, uKeyLen,
				pData, fDestroy);
      } else {
	// Append
	(*ppItem)->pRight= trie_item_create(uKey, uKeyLen, pData);
	return 0;
      }
    } else {
      if ((*ppItem)->pLeft != NULL) {
	// Recurse
	return trie_item_insert(&(*ppItem)->pLeft, uKey, uKeyLen,
				pData, fDestroy);
      } else {
	// Append
	(*ppItem)->pLeft= trie_item_create(uKey, uKeyLen, pData);
	return 0;
      }
    }
  }
  return -1;
}

// -----[ trie_insert ]----------------------------------------------
/**
 * Insert one (key, value) pair into the Patricia tree.
 *
 * Pre: (key length < TRIE_KEY_SIZE)
 */
int trie_insert(STrie * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen,
		void * pData)
{
  if (pTrie->pRoot == NULL) {
    pTrie->pRoot= trie_item_create(uKey, uKeyLen, pData);
    return 0;
  } else {
    return trie_item_insert(&pTrie->pRoot, uKey, uKeyLen,
			    pData, pTrie->fDestroy);
  }
}

// -----[ trie_find_exact ]------------------------------------------
void * trie_find_exact(STrie * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen)
{
  STrieItem * pTemp;
  trie_key_t tPrefix;
  trie_key_len_t tPrefixLen;

  // Mask the given key according to its length
  uKey= (uKey & trie_predef_masks[uKeyLen]);

  if (pTrie->pRoot == NULL)
    return NULL;
  pTemp= pTrie->pRoot;
  while (pTemp != NULL) {

    // requested key is smaller than current => no match found
    if (uKeyLen < pTemp->uKeyLen)
      return NULL;

    // requested key has same length
    if (uKeyLen == pTemp->uKeyLen) {
      // (keys are equal) <=> match found
      if (uKey == pTemp->uKey)
	return pTemp->pData;
      else
	return NULL;
    }

    // requested key is longer => check if common parts match
    if (uKeyLen > pTemp->uKeyLen) {
      longest_common_prefix(pTemp->uKey, pTemp->uKeyLen,
			    uKey, uKeyLen, &tPrefix, &tPrefixLen);

      // Current key is too long => no match found
      if (tPrefixLen < pTemp->uKeyLen)
	return NULL;

      if (uKey & (1 << (TRIE_KEY_SIZE-tPrefixLen-1)))
	pTemp= pTemp->pRight;
      else
	pTemp= pTemp->pLeft;
    }
  }
  return NULL;
}

// -----[ trie_find_best ]-------------------------------------------
void * trie_find_best(STrie * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen)
{
  STrieItem * pTemp;
  void * pUpperData;
  trie_key_t tPrefix;
  trie_key_len_t tPrefixLen;

  // Mask the given key according to its length
  uKey= (uKey & trie_predef_masks[uKeyLen]);

  if (pTrie->pRoot == NULL)
    return NULL;
  pTemp= pTrie->pRoot;
  pUpperData= NULL;
  while (pTemp != NULL) {

    // requested key is smaller than current => no match found
    if (uKeyLen < pTemp->uKeyLen)
      break;

    // requested key has same length
    if (uKeyLen == pTemp->uKeyLen) {
      // (keys are equal) <=> match found
      if (uKey == pTemp->uKey)
	return pTemp->pData;
      else
	break;
    }

    // requested key is longer => check if common parts match
    if (uKeyLen > pTemp->uKeyLen) {
      longest_common_prefix(pTemp->uKey, pTemp->uKeyLen,
			    uKey, uKeyLen, &tPrefix, &tPrefixLen);

      // Current key is too long => no match found
      if (tPrefixLen < pTemp->uKeyLen)
	break;

      pUpperData= pTemp->pData;
      if (uKey & (1 << (TRIE_KEY_SIZE-tPrefixLen-1)))
	pTemp= pTemp->pRight;
      else
	pTemp= pTemp->pLeft;
    }
  }
  return pUpperData;
}

// -----[ trie_item_remove ]-----------------------------------------
/**
 *
 */
static int trie_item_remove(STrieItem ** ppItem, trie_key_t uKey,
			    trie_key_len_t uKeyLen, FTrieDestroy fDestroy)
{
  STrieItem * pTemp;
  trie_key_t tPrefix;
  trie_key_len_t tPrefixLen;
  int iResult;

  // requested key is smaller than current => no match found
  if (uKeyLen < (*ppItem)->uKeyLen)
    return -1;

  // requested key has same length
  if (uKeyLen == (*ppItem)->uKeyLen) {
    if ((uKey == (*ppItem)->uKey) && ((*ppItem)->pData != NULL)) {

      if (fDestroy != NULL)
	fDestroy(&(*ppItem)->pData);
      (*ppItem)->pData= NULL;

      // Two cases: 2 childs or less
      if (((*ppItem)->pLeft != NULL) &&
	  ((*ppItem)->pRight != NULL)) {
	// Item can not be destroyed
      } else {
	// Item can be destroyed and replaced by the non-null child
	pTemp= *ppItem;
	if ((*ppItem)->pLeft != NULL)
	  *ppItem= (*ppItem)->pLeft;
	else
	  *ppItem= (*ppItem)->pRight;
	FREE(pTemp);
      }

      return 0;
    } else
      return -1;
  }

  // requested key is longer => check if common parts match
  if (uKeyLen > (*ppItem)->uKeyLen) {
    longest_common_prefix((*ppItem)->uKey, (*ppItem)->uKeyLen,
			  uKey, uKeyLen, &tPrefix, &tPrefixLen);
    
    // Current key is too long => no match found
    if (tPrefixLen < (*ppItem)->uKeyLen)
      return -1;
    
    if (uKey & (1 << (TRIE_KEY_SIZE-tPrefixLen-1))) {
      if ((*ppItem)->pRight != NULL)
	iResult= trie_item_remove(&(*ppItem)->pRight, uKey, uKeyLen, fDestroy);
      else
	return -1;
    } else {
      if ((*ppItem)->pLeft != NULL)
	iResult= trie_item_remove(&(*ppItem)->pLeft, uKey, uKeyLen, fDestroy);
      else
	return -1;
    }

    // Need to propagate removal ?
    if ((iResult == 0) && ((*ppItem)->pData == NULL)) {
      // If the local value does not exist and if the local node has
      // less than 2 childs, it should be removed and replaced by its
      // child (if any).
      if (((*ppItem)->pLeft == NULL) || ((*ppItem)->pRight == NULL)) {
	pTemp= *ppItem;
	if ((*ppItem)->pLeft != NULL)
	  *ppItem= (*ppItem)->pLeft;
	else
	  *ppItem= (*ppItem)->pRight;
	FREE(pTemp);
      }
    }
    return iResult;
  }  
  return -1;
}

// -----[ trie_remove ]----------------------------------------------
/**
 * Remove the value associated with the given key. Remove any
 * unnecessary nodes in the tree.
 *
 * Pre: (key length < TRIE_KEY_SIZE)
 *
 * Result: -1 if key did not exist. 0 if key has been removed.
 */
int trie_remove(STrie * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen)
{
  // Mask the given key according to its length
  uKey= uKey & trie_predef_masks[uKeyLen];

  if (pTrie->pRoot != NULL)
    return trie_item_remove(&pTrie->pRoot, uKey, uKeyLen, pTrie->fDestroy);
  else
    return -1;
}

// -----[ trie_item_replace ]----------------------------------------
static int trie_item_replace(STrieItem * pItem, trie_key_t uKey,
			     trie_key_len_t uKeyLen, void * pData)
{
  trie_key_t tPrefix;
  trie_key_len_t tPrefixLen;

  // requested key is smaller than current => no match found
  if (uKeyLen < pItem->uKeyLen)
    return -1;

  // requested key has same length
  if (uKeyLen == pItem->uKeyLen) {
    if ((uKey == pItem->uKey) && (pItem->pData != NULL)) {
      pItem->pData= pData;
      return 0;
    } else
      return -1;
  }

  // requested key is longer => check if common parts match
  if (uKeyLen > pItem->uKeyLen) {
    longest_common_prefix(pItem->uKey, pItem->uKeyLen,
			  uKey, uKeyLen, &tPrefix, &tPrefixLen);
    
    // Current key is too long => no match found
    if (tPrefixLen < pItem->uKeyLen)
      return -1;
    
    if (uKey & (1 << (TRIE_KEY_SIZE-tPrefixLen-1))) {
      if (pItem->pRight != NULL)
	return trie_item_replace(pItem->pRight, uKey, uKeyLen, pData);
      else
	return -1;
    } else {
      if (pItem->pLeft != NULL)
	return trie_item_replace(pItem->pLeft, uKey, uKeyLen, pData);
      else
	return -1;
    }
  }
  return -1;
}

// -----[ trie_replace ]---------------------------------------------
int trie_replace(STrie * pTrie, trie_key_t uKey,
		 trie_key_len_t uKeyLen, void * pData)
{
  // Mask the given key according to its length
  uKey= uKey & trie_predef_masks[uKeyLen];

  if (pTrie->pRoot != NULL)
    return trie_item_replace(pTrie->pRoot, uKey, uKeyLen, pData);
  else
    return -1;  
}

// -----[ trie_item_destroy ]----------------------------------------
static void trie_item_destroy(STrieItem ** ppItem, FTrieDestroy fDestroy)
{
  if (*ppItem != NULL) {
    if ((fDestroy != NULL) && ((*ppItem)->pData != NULL))
      fDestroy(&(*ppItem)->pData);
    if ((*ppItem)->pLeft != NULL)
      trie_item_destroy(&(*ppItem)->pLeft, fDestroy);
    if ((*ppItem)->pRight != NULL)
      trie_item_destroy(&(*ppItem)->pRight, fDestroy);
    FREE(*ppItem);
  }
}

// -----[ trie_destroy ]---------------------------------------------
void trie_destroy(STrie ** ppTrie)
{
  if (*ppTrie != NULL) {
    trie_item_destroy(&(*ppTrie)->pRoot, (*ppTrie)->fDestroy);
    FREE(*ppTrie);
    *ppTrie= NULL;
  }
}

// -----[ trie_item_for_each ]---------------------------------------
static int trie_item_for_each(STrieItem * pItem,
			       FTrieForEach fForEach, void * pContext)
{
  int iResult;

  if (pItem->pLeft != NULL) {
    iResult= trie_item_for_each(pItem->pLeft, fForEach, pContext);
    if (iResult != 0)
      return iResult;
  }
  if (pItem->pRight != NULL) {
    iResult= trie_item_for_each(pItem->pRight, fForEach, pContext);
    if (iResult != 0)
      return iResult;
  }

  if (pItem->pData != NULL)
    return fForEach(pItem->uKey, pItem->uKeyLen, pItem->pData, pContext);
  else
    return 0;
}

// -----[ trie_for_each ]--------------------------------------------
int trie_for_each(STrie * pTrie, FTrieForEach fForEach, void * pContext)
{
  if (pTrie->pRoot != NULL)
    return trie_item_for_each(pTrie->pRoot, fForEach, pContext);
  return 0;
}

/////////////////////////////////////////////////////////////////////
//
// ENUMERATION
//
/////////////////////////////////////////////////////////////////////

// -----[ _trie_get_array_for_each ]---------------------------------
int _trie_get_array_for_each(uint32_t uKey, uint8_t uKeyLen,
			     void * pItem, void * pContext)
{
  SPtrArray * pArray= (SPtrArray *) pContext;
  if (ptr_array_append(pArray, pItem) < 0)
    return -1;
  return 0;
}

// -----[ trie_get_array ]-------------------------------------------
SPtrArray * trie_get_array(STrie * pTrie)
{
  SPtrArray * pArray= ptr_array_create_ref(0);
  if (trie_for_each(pTrie,
		    _trie_get_array_for_each,
		    pArray)) {
    ptr_array_destroy(&pArray);
    pArray= NULL;
  }
  return pArray;
}

// ----- STrieEnumContext -------------------------------------------
typedef struct {
  SPtrArray * pArray;
  SEnumerator * pEnum;
} STrieEnumContext;

// -----[ _trie_get_enum_has_next ]----------------------------------
int _trie_get_enum_has_next(void * pContext)
{
  STrieEnumContext * pTrieContext= (STrieEnumContext *) pContext;
  return enum_has_next(pTrieContext->pEnum);
}

// -----[ _trie_get_enum_get_next ]----------------------------------
void * _trie_get_enum_get_next(void * pContext)
{
  STrieEnumContext * pTrieContext= (STrieEnumContext *) pContext;
  return enum_get_next(pTrieContext->pEnum);
}

// -----[ _trie_get_enum_destroy ]-----------------------------------
void _trie_get_enum_destroy(void * pContext)
{
  STrieEnumContext * pTrieContext= (STrieEnumContext *) pContext;
  enum_destroy(&pTrieContext->pEnum);
  ptr_array_destroy(&pTrieContext->pArray);
  FREE(pTrieContext);
}

// -----[ trie_get_enum ]--------------------------------------------
SEnumerator * trie_get_enum(STrie * pTrie)
{
  STrieEnumContext * pContext=
    (STrieEnumContext *) MALLOC(sizeof(STrieEnumContext));
  pContext->pArray= trie_get_array(pTrie);
  pContext->pEnum= _array_get_enum((SArray *) pContext->pArray);

  return enum_create(pContext,
		     _trie_get_enum_has_next,
		     _trie_get_enum_get_next,
		     _trie_get_enum_destroy);
}

/////////////////////////////////////////////////////////////////////
//
// INITIALIZATION PART
//
/////////////////////////////////////////////////////////////////////

// -----[ patricia_tree_init ]---------------------------------------
void _patricia_tree_init()
{
  _trie_init();
}
