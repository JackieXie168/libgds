// ==================================================================
// @(#)dllist.c
//
// Doubly-linked lists
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 26/07/2006
// @lastdate 30/07/2006
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libgds/dllist.h>
#include <libgds/memory.h>

// -----[ dllist_item_create ]---------------------------------------
SDLListItem * dllist_item_create(void * pUserData,
				 SDLListItem * pPrevious,
				 SDLListItem * pNext)
{
  SDLListItem * pItem= (SDLListItem *) MALLOC(sizeof(SDLListItem));
  pItem->pUserData= pUserData;
  pItem->pPrevious= pPrevious;
  pItem->pNext= pNext;
  return pItem;
}

// -----[ dllist_item_destroy ]--------------------------------------
void dllist_item_destroy(SDLListItem ** ppItem, FDLListDestroy fDestroy)
{
  if (*ppItem != NULL) {
    if (fDestroy != NULL)
      fDestroy(ppItem);
    FREE(*ppItem);
    *ppItem= NULL;
  }
}

// -----[ dllist_create ]--------------------------------------------
SDLList * dllist_create(FDLListDestroy fDestroy)
{
  SDLList * pList= (SDLList *) MALLOC(sizeof(SDLList));
  pList->pRoot= NULL;
  pList->fDestroy= fDestroy;
  return pList;
}

// -----[ dllist_destroy ]-------------------------------------------
void dllist_destroy(SDLList ** ppList)
{
  SDLListItem * pItem, *pItemTmp;

  if (*ppList != NULL) {
    pItem= (*ppList)->pRoot;
    while (pItem != NULL) {
      pItemTmp= pItem;
      pItem= pItem->pNext;
      dllist_item_destroy(&pItemTmp, (*ppList)->fDestroy);
    }
    FREE(*ppList);
    *ppList= NULL;
  }
}

// -----[ dllist_insert ]--------------------------------------------
/**
 * Return value:
 *   0 success
 *   -1 invalid index
 */
int dllist_insert(SDLList * pList, unsigned int uIndex, void * pUserData)
{
  SDLListItem * pItem;

  if (uIndex == 0) {
    pList->pRoot= dllist_item_create(pUserData, NULL, pList->pRoot);
  } else {
    pItem= pList->pRoot;
    while ((pItem != NULL) && (uIndex > 1)) {
      pItem= pItem->pNext;
      uIndex--;
    }
    if (pItem != NULL) {
      pItem->pNext= dllist_item_create(pUserData, pItem, pItem->pNext);
    } else {
      return -1;
    }
  }
  return 0;
}

// -----[ dllist_remove ]--------------------------------------------
/**
 * Return value:
 *   0  success
 *   -1 invalid index
 */
int dllist_remove(SDLList * pList, unsigned int uIndex)
{
  SDLListItem * pTmpItem; // Pointer to item to delete
  SDLListItem ** ppCurrItem; // Pointer to pointer of item to delete

  // Not even a single item, abort with error
  if (pList->pRoot == NULL)
    return -1;

  ppCurrItem= &(pList->pRoot);
  while ((uIndex > 0) && (*ppCurrItem != NULL)) {
    // Note: advance pointer to pointer
    ppCurrItem= &(*ppCurrItem)->pNext;
    uIndex--;
  }
  if (*ppCurrItem == NULL)
    return -1;

  // Keep reference to current item
  pTmpItem= *ppCurrItem;

  // Update cross-references
  if ((*ppCurrItem)->pNext != NULL)
    (*ppCurrItem)->pNext->pPrevious= (*ppCurrItem)->pPrevious;
  *ppCurrItem= (*ppCurrItem)->pNext;

  // Destroy temporary item
  dllist_item_destroy(&pTmpItem, pList->fDestroy);

  return 0;
}

// -----[ dllist_append ]--------------------------------------------
/**
 */
int dllist_append(SDLList * pList, void * pUserData)
{
  SDLListItem * pItem;

  if (pList->pRoot == NULL) {
    pList->pRoot= dllist_item_create(pUserData, NULL, NULL);
  } else {
    pItem= pList->pRoot;
    while (pItem->pNext != NULL) {
      pItem= pItem->pNext;
    }
    pItem->pNext= dllist_item_create(pUserData, pItem, NULL);
  }
  return 0;
}

// -----[ dllist_get ]-----------------------------------------------
int dllist_get(SDLList * pList, unsigned int uIndex, void ** ppUserData)
{
  SDLListItem * pItem= pList->pRoot;

  while ((uIndex > 0) && (pItem != NULL)) {
    pItem= pItem->pNext;
    uIndex--;
  }
  if (pItem == NULL)
    return -1;
  *ppUserData= pItem->pUserData;
  return 0;
}

// -----[ dllist_size ]-----------------------------------------------
unsigned int dllist_size(SDLList * pList)
{
  SDLListItem * pItem= pList->pRoot;
  unsigned int uSize= 0;

  while (pItem != NULL) {
    pItem= pItem->pNext;
    uSize++;
  }
  return uSize;
}

// -----[ dllist_for_each ]------------------------------------------
int dllist_for_each(SDLList * pList, void * pContext,
		    FDLListForEach fForEach)
{
  SDLListItem * pItem= pList->pRoot;
  int iResult;

  while (pItem != NULL) {
    iResult= fForEach(pItem->pUserData, pContext);
    if (iResult != 0)
      return iResult;
    pItem= pItem->pNext;
  }
  return 0;
}
