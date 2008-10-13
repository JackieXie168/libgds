// ==================================================================
// @(#)fifo.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Damien Saucez (damien.saucez@uclouvain.be)
// @date 28/11/2002
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <libgds/fifo.h>
#include <libgds/memory.h>

// ----- fifo_create ------------------------------------------------
/**
 *
 */
gds_fifo_t * fifo_create(uint32_t max_depth, FFIFODestroy destroy)
{
  gds_fifo_t * fifo= (gds_fifo_t *) MALLOC(sizeof(gds_fifo_t));
  fifo->options= 0;
  fifo->max_depth= max_depth;
  fifo->start_index= 0;
  fifo->current_depth= 0;
  fifo->fDestroy= destroy;
  fifo->items= (void **) MALLOC(sizeof(void *)*max_depth);
  return fifo;
}

// ----- fifo_destroy -----------------------------------------------
/**
 *
 */
void fifo_destroy(gds_fifo_t ** fifo_ref)
{
  gds_fifo_t * fifo= *fifo_ref;
  unsigned int index;

  if (fifo != NULL) {
    if (fifo->fDestroy != NULL)
      for (index= 0; index < fifo->current_depth; index++)
	fifo->fDestroy(&fifo->items[(fifo->start_index+index) %
				      fifo->max_depth]);
    FREE(fifo->items);
    fifo->items= NULL;
    FREE(fifo);
    *fifo_ref= NULL;
  }
}

// ----- fifo_set_option ------------------------------------------
/**
 *
 */
void fifo_set_option(gds_fifo_t * fifo, uint8_t option, int state)
{
  if (state)
    fifo->options|= option;
  else
    fifo->options&= ~option;
}

// ----- _fifo_grow -----------------------------------------------
/**
 *
 */
static int _fifo_grow(gds_fifo_t * fifo)
{
  uint32_t new_depth= 0;

  // Note: currently, whatever exponential or linear is selected,
  // _fifo_grow will lead to exponential growth
  if (fifo->options & FIFO_OPTION_GROW_EXPONENTIAL)
    new_depth= fifo->max_depth * 2;
  else if (fifo->options & FIFO_OPTION_GROW_LINEAR)
    new_depth= fifo->max_depth * 2;
  else
    return -1;

  if (new_depth > fifo->max_depth) {

    // Re-allocate FIFO space
    fifo->items= REALLOC(fifo->items,
			   (sizeof(void *)*new_depth));

    // Move exiting items
    if (fifo->current_depth > fifo->max_depth-fifo->start_index)
      memcpy(&fifo->items[fifo->max_depth], &fifo->items[0],
	     (fifo->current_depth-(fifo->max_depth-fifo->start_index))*
	     sizeof(void *));
    
    fifo->max_depth= new_depth;
    
  }
  return 0;
}

// ----- fifo_push --------------------------------------------------
/**
 *
 */
int fifo_push(gds_fifo_t * fifo, void * item)
{

  // If there is not enough space in the queue, try to grow it
  if (fifo->current_depth >= fifo->max_depth)
    if (_fifo_grow(fifo) != 0)
      return -1;

  fifo->items[(fifo->start_index+
		  fifo->current_depth) % fifo->max_depth]= item;
  fifo->current_depth++;
  return 0;
}

// ----- fifo_pop ---------------------------------------------------
/**
 *
 */
void * fifo_pop(gds_fifo_t * fifo)
{
  void * item= NULL;

  if (fifo->current_depth > 0) {
    item= fifo->items[fifo->start_index];
    fifo->start_index= (fifo->start_index+1) % fifo->max_depth;
    fifo->current_depth--;
  }
  return item;
}

// -----[ fifo_depth ]---------------------------------------------
/**
 *
 */
uint32_t fifo_depth(gds_fifo_t * fifo)
{
  return fifo->current_depth;
}

// ----- fifo_get_at ------------------------------------------------
/**
 * Get the iPos'th element in the queue fifo.
 * Return the element at the iPos position in the queue. If iPos
 * is out of the bounds of the queue, a NULL is returned.
 * PARAM:  fifo: the queue
 * 	   iPos:  position in the queue (0 is the head of the queue)
 * RETURN: If iPos valid, element at position iPos in fifo is returned.
 *	   Otherwise a NULL is returned.
 */
void * fifo_get_at(gds_fifo_t * fifo, unsigned int pos)
{
  if (fifo && (pos < fifo->current_depth))
    return fifo->items[(fifo->start_index + pos) %
			 fifo->max_depth];
  return NULL;
}
