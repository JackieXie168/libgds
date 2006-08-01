// ==================================================================
// @(#)dllist.h
//
// Doubly-linked lists
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 26/07/2006
// @lastdate 29/07/2006
// ==================================================================

#ifndef __DLLIST_H__
#define __DLLIST_H__

// -----[ SDLListItem ]----------------------------------------------
struct TDLListItem {
  struct TDLListItem * pPrevious;
  struct TDLListItem * pNext;
  void * pUserData;
};
typedef struct TDLListItem SDLListItem;

// -----[ FDLListForEach ]-------------------------------------------
typedef int (* FDLListForEach)(void * pUserData, void * pContext);
// -----[ FDLListDestroy ]-------------------------------------------
typedef void (* FDLListDestroy)(SDLListItem ** ppItem);

// -----[ SDLList ]--------------------------------------------------
typedef struct {
  SDLListItem * pRoot;
  FDLListDestroy fDestroy;
} SDLList;

// -----[ dllist_create ]--------------------------------------------
extern SDLList * dllist_create();
// -----[ dllist_destroy ]-------------------------------------------
extern void dllist_destroy(SDLList ** ppList);
// -----[ dllist_insert ]--------------------------------------------
extern int dllist_insert(SDLList * pList, unsigned int uIndex,
			 void * pUserData);
// -----[ dllist_remove ]--------------------------------------------
extern int dllist_remove(SDLList * pList, unsigned int uIndex);
// -----[ dllist_append ]--------------------------------------------
extern int dllist_append(SDLList * pList, void * pUserData);
// -----[ dllist_get ]-----------------------------------------------
extern int dllist_get(SDLList * pList, unsigned int uIndex,
		      void ** ppUserData);
// -----[ dllist_size ]-----------------------------------------------
extern unsigned int dllist_size(SDLList * pList);
// -----[ dllist_for_each ]------------------------------------------
extern int dllist_for_each(SDLList * pList, void * pContext,
			   FDLListForEach fForEach);

#endif /* __DLLIST_H__ */
