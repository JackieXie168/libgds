// ==================================================================
// @(#)list.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be), Sebastien Tandel
// @date 23/11/2002
// @lastdate 13/06/2002
// ==================================================================

#ifndef __LIST_H__
#define __LIST_H__

// ----- FListCompare -----------------------------------------------
/**
 * ITEM1 == ITEM2 => 0
 * ITEM1 > ITEM2 => +1
 * ITEM1 < ITEM2 => -1
 */
typedef int (*FListCompare)(void * pItem1, void * pItem2);

// ----- FListDestroy -----------------------------------------------
typedef void (*FListDestroy)(void ** ppItem);

// ----- FListForEach -----------------------------------------------
typedef void (*FListForEach)(void * pItem, void * pContext);

// ----- FListCopyItem ----------------------------------------------
typedef void * (*FListCopyItem)(void * pItem);

// ----- SList ------------------------------------------------------
typedef struct {
  int iSize;
  unsigned int uNbrElt;
  unsigned int uStepResize;
  void ** ppItems;
  FListCompare fCompare;
  FListDestroy fDestroy;
} SList;

// ----- list_create ------------------------------------------------
extern SList * list_create(FListCompare fCompare,
			   FListDestroy fDestroy, unsigned int uStepResize);
// ----- list_destroy -----------------------------------------------
extern void list_destroy(SList ** ppList);
// ----- list_find_index --------------------------------------------
extern int list_find_index(SList * pList, void * pItem,
			   int * piIndex);
// ----- list_insert_index ------------------------------------------
extern int list_insert_index(SList * pList, int iIndex,
			     void * pItem);
// ----- list_get_nbr_element ---------------------------------------
int list_get_nbr_element(SList * pList);
// ----- list_get_index ---------------------------------------------
void * list_get_index(SList * pList, int iIndex);
// ----- list_add ---------------------------------------------------
extern int list_add(SList * pList, void * pItem);
// ----- list_delete ------------------------------------------------
int list_delete(SList * pList, int iIndex);
// ----- list_replace -----------------------------------------------
extern int list_replace(SList * pList, int iIndex, void * pItem);
// ----- list_for_each ----------------------------------------------
extern void list_for_each(SList * pList, FListForEach fForEach,
			  void * pContext);
// ----- list_copy --------------------------------------------------
extern SList * list_copy(SList * pList, FListCopyItem fCopyItem);

#endif
