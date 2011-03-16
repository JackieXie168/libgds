// ==================================================================
// @(#)fifo.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Damien Saucez (damien.saucez@uclouvain.be)
// @date 28/11/2002
// $Id: fifo.c 306 2009-03-27 11:56:16Z bquoitin $
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <libgds/fifo_tunable.h>
#include <libgds/memory.h>

// ----- fifo_create ------------------------------------------------
/**
 *
 */
gds_fifo_tunable_t * fifo_tunable_create(unsigned int max_depth, gds_fifo_destroy_f destroy)
{
  gds_fifo_tunable_t * fifo= (gds_fifo_tunable_t *) MALLOC(sizeof(gds_fifo_tunable_t));
  fifo->options= 0;
  fifo->max_depth= max_depth;
  fifo->start_index= 0;
  fifo->current_depth= 0;
  fifo->destroy= destroy;
  fifo->items= (void **) MALLOC(sizeof(void *)*max_depth);
  return fifo;
}



// ----- fifo_destroy -----------------------------------------------
/**
 *
 */
void fifo_tunable_destroy(gds_fifo_tunable_t ** fifo_ref)
{
  gds_fifo_tunable_t * fifo= *fifo_ref;
  unsigned int index;

  if (fifo != NULL) {
    if (fifo->destroy != NULL)
      for (index= 0; index < fifo->current_depth; index++)
	fifo->destroy(&fifo->items[(fifo->start_index+index) %
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
void fifo_tunable_set_option(gds_fifo_tunable_t * fifo, uint8_t option, int state)
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
static int _fifo_tunable_grow(gds_fifo_tunable_t * fifo)
{
  unsigned int new_depth= 0;

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
int fifo_tunable_push(gds_fifo_tunable_t * fifo, void * item)
{

  // If there is not enough space in the queue, try to grow it
  if (fifo->current_depth >= fifo->max_depth)
    if (_fifo_tunable_grow(fifo) != 0)
      return -1;

  fifo->items[(fifo->start_index+
		  fifo->current_depth) % fifo->max_depth]= item;
  fifo->current_depth++;
  return 0;
}

int fifo_tunable_swap(gds_fifo_tunable_t * fifo, int nb1, int nb2)
{
      void * item1;

    printf("echanger le %d avec le %d\n",
            (fifo->start_index + nb1) % fifo->max_depth ,
            (fifo->start_index + nb2) % fifo->max_depth );

    item1 = fifo->items[(fifo->start_index + nb1) % fifo->max_depth];
    fifo->items[(fifo->start_index + nb1) % fifo->max_depth] = fifo->items[(fifo->start_index + nb2) % fifo->max_depth];
    fifo->items[(fifo->start_index + nb2) % fifo->max_depth] = item1;
    return 0;
}



/*
 set num as first, and decale others

 */
int fifo_tunable_bringForward(gds_fifo_tunable_t * fifo, int num)
{
      void * item;

    printf("Bring Forward the msg %d\n", num );


      item = fifo->items[(fifo->start_index + num) % fifo->max_depth];

      for(int i=num-1; i>=0 ; i--)
      {
          fifo->items[( fifo->start_index + i + 1 ) % fifo->max_depth] = fifo->items[( fifo->start_index + i ) % fifo->max_depth];
      }

    fifo->items[(fifo->start_index) % fifo->max_depth] = item;
    
    return 0;
}


int fifo_tunable_set_first(gds_fifo_tunable_t * fifo, int nb)
{
    //return fifo_tunable_swap(fifo, 0, nb );
    return fifo_tunable_bringForward(fifo,nb);

    /*

    //swap the items in current position, and indexOfNext
    void * thecurrent = fifo->items[(fifo->start_index+fifo->current_depth) % fifo->max_depth];
    fifo->items[(fifo->start_index+fifo->current_depth) % fifo->max_depth] =
            fifo->items[(indexOfNext) % fifo->max_depth];
    fifo->items[(indexOfNext) % fifo->max_depth]= thecurrent;
*/

}

// ----- fifo_pop ---------------------------------------------------
/**
 *
 */
void * fifo_tunable_pop(gds_fifo_tunable_t * fifo)
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
unsigned int fifo_tunable_depth(gds_fifo_tunable_t * fifo)
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
void * fifo_tunable_get_at(gds_fifo_tunable_t * fifo, unsigned int pos)
{
  if (fifo && (pos < fifo->current_depth))
    return fifo->items[(fifo->start_index + pos) %
			 fifo->max_depth];
  return NULL;
}



gds_fifo_tunable_t * fifo_tunable_copy(gds_fifo_tunable_t * fifo, gds_fifo_copy_item_f copy_item)
{
    if(fifo==NULL)
        return NULL;
    
    gds_fifo_tunable_t * new_fifo = fifo_tunable_create( fifo->max_depth, fifo->destroy);

    unsigned int i=0;
    while(i<fifo->current_depth)
    {
        void * item_copied = copy_item(   fifo_tunable_get_at(fifo, i)   );
        fifo_tunable_push( new_fifo, item_copied  );
        i++;
    }
    return new_fifo;
}
