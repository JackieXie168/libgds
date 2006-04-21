// ==================================================================
// @(#)stack.h
//
// Stack
//
// @author Bruno Quoitin
// @date 21/03/2003
// @lastdate 18/07/2003
// ==================================================================

#ifndef __STACK_H__
#define __STACK_H__

typedef struct {
  int iMaxDepth;
  int iDepth;
  void * apItems[0];
} SStack;

// ----- stack_create -----------------------------------------------
extern SStack * stack_create(int iMaxDepth);
// ----- stack_destroy ----------------------------------------------
extern void stack_destroy(SStack ** ppStack);
// ----- stack_push -------------------------------------------------
extern int stack_push(SStack * pStack, void * pItem);
// ----- stack_pop --------------------------------------------------
extern void * stack_pop(SStack * pStack);
// ----- stack_top --------------------------------------------------
extern void * stack_top(SStack * pStack);
// ----- stack_get_at -----------------------------------------------
extern void * stack_get_at(SStack * pStack, unsigned int iIndex);
// ----- stack_depth ------------------------------------------------
extern int stack_depth(SStack * pStack);
// ----- stack_is_empty ---------------------------------------------
extern int stack_is_empty(SStack * pStack);
// ----- stack_copy -------------------------------------------------
extern SStack * stack_copy(SStack * pStack);
// ----- stack_compare ----------------------------------------------
extern int stack_equal(SStack * pStack1, SStack * pStack2);

#endif
