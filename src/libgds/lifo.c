
// ==================================================================
// @(#)lifo.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Damien Saucez (damien.saucez@uclouvain.be)
// @date 28/11/2002
// $Id: lifo.c 306 2009-03-27 11:56:16Z bquoitin $
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <libgds/lifo.h>
#include <libgds/memory.h>

// ----- lifo_create ------------------------------------------------
/**
 *
 */
gds_lifo_t * lifo_create(unsigned int max_depth, gds_lifo_destroy_f destroy)
{
  gds_lifo_t * lifo= (gds_lifo_t *) MALLOC(sizeof(gds_lifo_t));
  lifo->options= 0;
  lifo->max_depth= max_depth;
  lifo->current_depth= 0;
  lifo->destroy= destroy;
  lifo->items= (void **) MALLOC(sizeof(void *)*max_depth);
  return lifo;
}

// ----- lifo_destroy -----------------------------------------------
/**
 *
 */
void lifo_destroy(gds_lifo_t ** lifo_ref)
{
  gds_lifo_t * lifo= *lifo_ref;
  unsigned int index;

  if (lifo != NULL) {
    if (lifo->destroy != NULL)
      for (index= 0; index < lifo->current_depth; index++)
	lifo->destroy(&lifo->items[(index) %
				   lifo->max_depth]);
    FREE(lifo->items);
    lifo->items= NULL;
    FREE(lifo);
    *lifo_ref= NULL;
  }
}

// ----- lifo_set_option ------------------------------------------
/**
 *
 */
void lifo_set_option(gds_lifo_t * lifo, uint8_t option, int state)
{
  if (state)
    lifo->options|= option;
  else
    lifo->options&= ~option;
}

// ----- _lifo_grow -----------------------------------------------
/**
 *
 */
static int _lifo_grow(gds_lifo_t * lifo)
{
  unsigned int new_depth= 0;

  // Note: currently, whatever exponential or linear is selected,
  // _lifo_grow will lead to exponential growth
  if (lifo->options & LIFO_OPTION_GROW_EXPONENTIAL)
    new_depth= lifo->max_depth * 2;
  else if (lifo->options & LIFO_OPTION_GROW_LINEAR)
    new_depth= lifo->max_depth * 2;
  else
    return -1;

  if (new_depth > lifo->max_depth) {

    // Re-allocate lifo space
    lifo->items= REALLOC(lifo->items,
			   (sizeof(void *)*new_depth));

    // Move exiting items
    if (lifo->current_depth > lifo->max_depth)
      memcpy(&lifo->items[lifo->max_depth], &lifo->items[0],
	     (lifo->current_depth-(lifo->max_depth))*
	     sizeof(void *));

    lifo->max_depth= new_depth;

  }
  return 0;
}

// ----- lifo_push --------------------------------------------------
/**
 *
 */
int lifo_push(gds_lifo_t * lifo, void * item)
{

  // If there is not enough space in the lifo, try to grow it
  if (lifo->current_depth >= lifo->max_depth)
    if (_lifo_grow(lifo) != 0)
      return -1;

  lifo->items[(  lifo->current_depth) % lifo->max_depth]= item;
  lifo->current_depth++;
  return 0;
}

// ----- lifo_pop ---------------------------------------------------
/**
 *
 */
void * lifo_pop(gds_lifo_t * lifo)
{
  void * item= NULL;

  if (lifo->current_depth > 0) {
    item= lifo->items[lifo->current_depth];
    lifo->current_depth--;
  }
  return item;
}

// -----[ lifo_depth ]---------------------------------------------
/**
 *
 */
unsigned int lifo_depth(gds_lifo_t * lifo)
{
  return lifo->current_depth;
}

// ----- lifo_get_at ------------------------------------------------
/**
 * Get the iPos'th element in the queue lifo.
 * Return the element at the iPos position in the queue. If iPos
 * is out of the bounds of the queue, a NULL is returned.
 * PARAM:  lifo: the queue
 * 	   iPos:  position in the queue (0 is the head of the queue)
 * RETURN: If iPos valid, element at position iPos in lifo is returned.
 *	   Otherwise a NULL is returned.
 */
void * lifo_get_at(gds_lifo_t * lifo, unsigned int pos)
{
  if (lifo && (pos < lifo->current_depth))
    return lifo->items[ pos %
			 lifo->max_depth];
  return NULL;
}
