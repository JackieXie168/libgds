// ==================================================================
// @(#)stack.h
//
// Stack
//
// @author Bruno Quoitin
// @date 21/03/2003
// @lastdate 04/12/2007
// ==================================================================

#ifndef __STACK_H__
#define __STACK_H__

typedef struct {
  int iMaxDepth;
  int iDepth;
  void * apItems[0];
} SStack;

#ifdef __cplusplus
extern "C" {
#endif
  
  // ----- stack_create ---------------------------------------------
  SStack * stack_create(int iMaxDepth);
  // ----- stack_destroy --------------------------------------------
  void stack_destroy(SStack ** ppStack);
  // ----- stack_push -----------------------------------------------
  int stack_push(SStack * pStack, void * pItem);
  // ----- stack_pop ------------------------------------------------
  void * stack_pop(SStack * pStack);
  // ----- stack_top ------------------------------------------------
  void * stack_top(SStack * pStack);
  // ----- stack_get_at ---------------------------------------------
  void * stack_get_at(SStack * pStack, unsigned int iIndex);
  // ----- stack_depth ----------------------------------------------
  int stack_depth(SStack * pStack);
  // ----- stack_is_empty -------------------------------------------
  int stack_is_empty(SStack * pStack);
  // ----- stack_copy -----------------------------------------------
  SStack * stack_copy(SStack * pStack);
  // ----- stack_compare --------------------------------------------
  int stack_equal(SStack * pStack1, SStack * pStack2);

#ifdef __cplusplus
}
#endif

#endif /* __STACK_H__ */
