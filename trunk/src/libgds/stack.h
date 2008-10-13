// ==================================================================
// @(#)stack.h
//
// Stack
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 21/03/2003
// $Id$
// ==================================================================

#ifndef __STACK_H__
#define __STACK_H__

typedef struct {
  int    max_depth;
  int    depth;
  void * items[0];
} gds_stack_t;

#ifdef __cplusplus
extern "C" {
#endif
  
  // ----- stack_create ---------------------------------------------
  gds_stack_t * stack_create(int max_depth);
  // ----- stack_destroy --------------------------------------------
  void stack_destroy(gds_stack_t ** stack_ref);
  // ----- stack_push -----------------------------------------------
  int stack_push(gds_stack_t * stack, void * data);
  // ----- stack_pop ------------------------------------------------
  void * stack_pop(gds_stack_t * stack);
  // ----- stack_top ------------------------------------------------
  void * stack_top(gds_stack_t * stack);
  // ----- stack_get_at ---------------------------------------------
  void * stack_get_at(gds_stack_t * stack, unsigned int index);
  // ----- stack_depth ----------------------------------------------
  int stack_depth(gds_stack_t * stack);
  // ----- stack_is_empty -------------------------------------------
  int stack_is_empty(gds_stack_t * stack);
  // ----- stack_copy -----------------------------------------------
  gds_stack_t * stack_copy(gds_stack_t * stack);
  // ----- stack_equal ----------------------------------------------
  int stack_equal(gds_stack_t * stack1, gds_stack_t * stack2);

#ifdef __cplusplus
}
#endif

#endif /* __STACK_H__ */
