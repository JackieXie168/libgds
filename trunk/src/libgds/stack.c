// ==================================================================
// @(#)stack.c
//
// Stack
//
// @author Bruno Quoitin
// @date 21/03/2003
// @lastdate 18/07/2003
// ==================================================================

#include <libgds/memory.h>
#include <libgds/stack.h>

// ----- stack_create -----------------------------------------------
/**
 *
 */
SStack * stack_create(int iMaxDepth)
{
  SStack * pStack= (SStack *) MALLOC(sizeof(SStack)+
				      iMaxDepth*sizeof(void *));
  pStack->iMaxDepth= iMaxDepth;
  pStack->iDepth= 0;
  return pStack;
}

// ----- stack_destroy ----------------------------------------------
/**
 *
 */
void stack_destroy(SStack ** ppStack)
{
  if (*ppStack != NULL) {
    FREE(*ppStack);
    *ppStack= NULL;
  }
}

// ----- stack_push -------------------------------------------------
/**
 *
 */
int stack_push(SStack * pStack, void * pItem)
{
  if (pStack->iDepth < pStack->iMaxDepth) {
    pStack->apItems[pStack->iDepth]= pItem;
    pStack->iDepth++;
    return 1;
  }
  return 0;
}

// ----- stack_pop --------------------------------------------------
/**
 *
 */
void * stack_pop(SStack * pStack)
{
  if (pStack->iDepth > 0) {
    pStack->iDepth--;
    return pStack->apItems[pStack->iDepth];
  }
  return NULL;
}

// ----- stack_top --------------------------------------------------
/**
 *
 */
void * stack_top(SStack * pStack)
{
  if (pStack->iDepth > 0)
    return pStack->apItems[pStack->iDepth-1];
  return NULL;
}

// ----- stack_get_at -----------------------------------------------
/**
 *
 */
void * stack_get_at(SStack * pStack, unsigned int iIndex)
{
  if (iIndex < pStack->iDepth)
    return pStack->apItems[iIndex];
  return NULL;
}

// ----- stack_depth ------------------------------------------------
/**
 *
 */
int stack_depth(SStack * pStack)
{
  return pStack->iDepth;
}

// ----- stack_is_empty ---------------------------------------------
/**
 *
 */
int stack_is_empty(SStack * pStack)
{
  if (pStack->iDepth > 0)
    return 0;
  return 1;
}

// ----- stack_copy -------------------------------------------------
/**
 *
 */
SStack * stack_copy(SStack * pStack)
{
  SStack * pNewStack;
  int iIndex;

  pNewStack= stack_create(pStack->iMaxDepth);
  pNewStack->iDepth= pStack->iDepth;
  for (iIndex= 0; iIndex < pNewStack->iDepth; iIndex++)
    pNewStack->apItems[iIndex]= pStack->apItems[iIndex];
  return pNewStack;
}

// ----- stack_equal ------------------------------------------------
/**
 *
 */
int stack_equal(SStack * pStack1, SStack * pStack2)
{
  int iIndex;

  if (pStack1->iDepth != pStack2->iDepth)
    return 0;
  for (iIndex= 0; iIndex < pStack1->iDepth; iIndex++)
    if (pStack1->apItems[iIndex] != pStack2->apItems[iIndex])
      return 0;
  return 1;
}
