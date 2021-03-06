// ==================================================================
// @(#)stack.c
//
// Stack
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 21/03/2003
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgds/memory.h>
#include <libgds/stack.h>

// ----- stack_create -----------------------------------------------
/**
 *
 */
gds_stack_t * stack_create(int max_depth)
{
  gds_stack_t * stack= (gds_stack_t *) MALLOC(sizeof(gds_stack_t)+
				      max_depth*sizeof(void *));
  stack->max_depth= max_depth;
  stack->depth= 0;
  return stack;
}

// ----- stack_destroy ----------------------------------------------
/**
 *
 */
void stack_destroy(gds_stack_t ** stack_ref)
{
  if (*stack_ref != NULL) {
    FREE(*stack_ref);
    *stack_ref= NULL;
  }
}

// ----- stack_push -------------------------------------------------
/**
 * RETURNS:
 *    0 in case of success
 *   -1 in case of failure
 */
int stack_push(gds_stack_t * stack, void * pItem)
{
  if (stack->depth < stack->max_depth) {
    stack->items[stack->depth]= pItem;
    stack->depth++;
    return 0;
  }
  return -1;
}

// ----- stack_pop --------------------------------------------------
/**
 *
 */
void * stack_pop(gds_stack_t * stack)
{
  if (stack->depth > 0) {
    stack->depth--;
    return stack->items[stack->depth];
  }
  return NULL;
}

// ----- stack_top --------------------------------------------------
/**
 *
 */
void * stack_top(gds_stack_t * stack)
{
  if (stack->depth > 0)
    return stack->items[stack->depth-1];
  return NULL;
}

// ----- stack_get_at -----------------------------------------------
/**
 *
 */
void * stack_get_at(gds_stack_t * stack, unsigned int iIndex)
{
  if (iIndex < stack->depth)
    return stack->items[iIndex];
  return NULL;
}

// ----- stack_depth ------------------------------------------------
/**
 *
 */
int stack_depth(gds_stack_t * stack)
{
  return stack->depth;
}

// ----- stack_is_empty ---------------------------------------------
/**
 * RETURNS:
 *   0 if the stack is not empty
 *   1 if the stack is empty
 */
int stack_is_empty(gds_stack_t * stack)
{
  if (stack->depth > 0)
    return 0;
  return 1;
}

// ----- stack_copy -------------------------------------------------
/**
 *
 */
gds_stack_t * stack_copy(gds_stack_t * stack)
{
  gds_stack_t * new_stack;
  unsigned int index;

  new_stack= stack_create(stack->max_depth);
  new_stack->depth= stack->depth;
  for (index= 0; index < new_stack->depth; index++)
    new_stack->items[index]= stack->items[index];
  return new_stack;
}

// ----- stack_equal ------------------------------------------------
/**
 *
 */
int stack_equal(gds_stack_t * stack1, gds_stack_t * stack2)
{
  unsigned int index;

  if (stack1->depth != stack2->depth)
    return 0;
  for (index= 0; index < stack1->depth; index++)
    if (stack1->items[index] != stack2->items[index])
      return 0;
  return 1;
}
