// ==================================================================
// @(#)enumerator.c
//
// List enumerator object.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 10/08/2005
// @lastdate 10/08/2005
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <libgds/memory.h>
#include <libgds/enumerator.h>

// ----- enum_create ------------------------------------------------
SEnumerator * enum_create(void * pContext,
			  FEnumeratorHasNext fHasNext,
			  FEnumeratorGetNext fGetNext,
			  FEnumeratorDestroy fDestroy)
{
  SEnumerator * pEnum= (SEnumerator *) MALLOC(sizeof(SEnumerator));
  pEnum->pContext= pContext;
  pEnum->fHasNext= fHasNext;
  pEnum->fGetNext= fGetNext;
  pEnum->fDestroy= fDestroy;
  return pEnum;
}

// ----- enum_destroy -----------------------------------------------
void enum_destroy(SEnumerator ** ppEnum)
{
  if (*ppEnum != NULL) {
    if ((*ppEnum)->fDestroy != NULL)
      (*ppEnum)->fDestroy((*ppEnum)->pContext);
    FREE(*ppEnum);
    *ppEnum= NULL;
  }
}

// ----- enum_has_next ----------------------------------------------
int enum_has_next(SEnumerator * pEnum)
{
  return pEnum->fHasNext(pEnum->pContext);
}

// ----- enum_get_next ----------------------------------------------
void * enum_get_next(SEnumerator * pEnum)
{
  return pEnum->fGetNext(pEnum->pContext);
}
