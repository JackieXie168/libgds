// ==================================================================
// @(#)enumerator.h
//
// List enumerator object.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 10/08/2005
// @lastdate 10/08/2005
// ==================================================================

#ifndef __GDS_ENUMERATOR_H__
#define __GDS_ENUMERATOR_H__

typedef int (*FEnumeratorHasNext)(void * pContext);
typedef void * (*FEnumeratorGetNext)(void * pContext);
typedef void (*FEnumeratorDestroy)(void * pContext);

typedef struct {
  void * pContext;
  FEnumeratorHasNext fHasNext;
  FEnumeratorGetNext fGetNext;
  FEnumeratorDestroy fDestroy;
} SEnumerator;

// ----- enum_create ------------------------------------------------
extern SEnumerator * enum_create(void * pContext,
				 FEnumeratorHasNext fHasNext,
				 FEnumeratorGetNext fGetNext,
				 FEnumeratorDestroy fDestroy);
// ----- enum_destroy -----------------------------------------------
extern void enum_destroy(SEnumerator ** ppEnum);
// ----- enum_has_next ----------------------------------------------
extern int enum_has_next(SEnumerator * pEnum);
// ----- enum_get_next ----------------------------------------------
extern void * enum_get_next(SEnumerator * pEnum);

#endif /* __GDS_ENUMERATOR_H__ */
