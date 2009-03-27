// ==================================================================
// @(#)list.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @author Sebastien Tandel (sta@info.ucl.ac.be)
// @date 23/11/2002
// $Id$
// ==================================================================
// Warning: performance of 'list_add' are poor (due to realloc +
// memmove) but memory usage and research are good !!!
// 
// 13/06/2003:
//   - added the possibility to resize the list by uStepResize. it
//     improves the performance by reducing the number of realloc
//     done but the number of calls to memmove() remains the same !
// 09/12/2004: 
//   - bug fixes (?)
//   - This code is only used to make the garbage collector for
//     memory_debug.c. For this reason, it doesn't use the MALLOC,
//     FREE and REALLOC wrappers but directly uses malloc, free and
//     realloc. If you need to use a list or an array, choose the
//     array implementation instead.
// 18/03/2007:
//   - add assert(ptr != NULL) for each call to malloc() and realloc().
//   - _list_resize() is now a static function
//
// TODO:
//   - current code of list_add() is very inefficient. The calls to
//     list_find_index() and  list_insert_index() should be merged
//     in order to perform a single lookup !!!
//   - rename this collection. The name "list" is not appropriate,
//     it is rather a sorted list
//   - since this data structure is only used internally, its's API
//     (header files) should not be disclosed.
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgds/list.h>

// -----[ _list_cmp ]------------------------------------------------
static inline int _list_cmp(const void * item1, const void * item2)
{
  if (item1 == item2)
    return 0;
  if (item1 < item2)
    return -1;
  return 1;
}

// -----[ list_create ]----------------------------------------------
gds_list_t * list_create(gds_list_cmp_f cmp,
			 gds_list_destroy_f destroy,
			 unsigned int resize_step)
{
  gds_list_t * list= (gds_list_t *) malloc(sizeof(gds_list_t));
  assert(list != NULL);
  list->size= 0;
  list->length = 0;
  if (resize_step == 0) 
    resize_step= 1;
  list->resize_step= resize_step;

  list->ops.cmp    = cmp;
  list->ops.destroy= destroy;

  list->items= NULL;
  return list;
}

// -----[ list_destroy ]---------------------------------------------
void list_destroy(gds_list_t ** list_ref)
{
  gds_list_t * list= *list_ref;
  unsigned int index;
  
  if (list == NULL)
    return;

  if (list->items != NULL) {
    if (list->ops.destroy != NULL)
      for (index= 0; index < list->length; index++)
	list->ops.destroy(&list->items[index]);
    free(list->items);
  }
  free(list);
  *list_ref= NULL;
}

// -----[ list_index_of ]--------------------------------------------
/**
 * ITEM found => 0, INDEX of item
 * ITEM not found => -1, INDEX where insertion must occur
 */
int list_index_of(gds_list_t * list, void * item, unsigned int * index_ref)
{
  unsigned int offset= 0;
  unsigned int length= list->length;
  unsigned int pos= length/2;
  int cmp_res;

  while (length > 0) {
    cmp_res= ((list->ops.cmp != NULL)?
	      list->ops.cmp(list->items[pos], item):
	      _list_cmp(list->items[pos], item));
    if (cmp_res == 0) {
      *index_ref= pos;
      return 0;
    }
    if (cmp_res > 0) {
      if (pos <= offset)
	break;
      length= pos-offset;
      pos= offset+length/2;
    } else {
      if (offset+length-pos <= 0)
	break;
      length= offset+length-pos-1;
      offset= pos+1;
      pos= offset+length/2;
    }
  }
  *index_ref= pos;
  return -1;
}

// -----[ _list_resized_if_required ]--------------------------------
static inline void _list_resize_if_required(gds_list_t * list)
{
  if (list->items != NULL) {
    if (list->size == 0) {
      free(list->items);
      list->items = NULL;
    } else {
      list->items= realloc(list->items, sizeof(void *)*list->size);
      assert(list->items != NULL);
    }
  } else {
    if (list->size == 0)
      return;
    list->items= malloc(sizeof(void *)*list->size);
    assert(list->items != NULL);
  }
}

// -----[ list_insert_at ]-------------------------------------------
int list_insert_at(gds_list_t * list, unsigned int index, void * item)
{
  if (index > list->length)
    return -1;
  
  list->length++;
  if (list->length >= list->size) {
    list->size+= list->resize_step;
    _list_resize_if_required(list);
  }
  
  if (index < list->length-1)
    memmove(&list->items[index+1], &list->items[index],
	    sizeof(void *)*(list->length-index-1));
  list->items[index]= item;
  return index;
}

// -----[ list_length ]----------------------------------------------
unsigned int list_length(gds_list_t * list)
{
  return list->length;
}

// -----[ list_get_at ]----------------------------------------------
void * list_get_at(gds_list_t * list, unsigned int index)
{
  if (index >= list->length)
    return NULL;
  return list->items[index];
}

// -----[ list_add ]-------------------------------------------------
int list_add(gds_list_t * list, void * item)
{
  unsigned int index;

  if (list_index_of(list, item, &index) == 0)
    return -1;
  return list_insert_at(list, index, item);
}

// -----[ list_remove_at ]-------------------------------------------
int list_remove_at(gds_list_t * list, unsigned int index)
{
  if (index >= list->length)
    return -1;
  
  if (list->ops.destroy != NULL)
    list->ops.destroy(&list->items[index]);
  
  if (list->length-(index+1) != 0)
    memmove(&(list->items[index]), &(list->items[index+1]),
	    sizeof(void *)*(list->length-(index+1)));
  
  list->length--;
  if (list->resize_step <= list->size-list->length) {
    list->size-= list->resize_step;
    _list_resize_if_required(list);
  }
  return 0;
}

// -----[ list_replace_at ]------------------------------------------
int list_replace_at(gds_list_t * list, unsigned int index, void * item)
{
  if (index >= list->length)
    return -1;

  if (list->ops.destroy != NULL)
    list->ops.destroy(&list->items[index]);
  list->items[index]= item;
  return 0;
}

// -----[ list_for_each ]--------------------------------------------
int list_for_each(gds_list_t * list, gds_list_foreach_f foreach,
		  void * ctx)
{
  unsigned int index;
  int result;
  for (index= 0; index < list->length; index++) {
    result= foreach(list->items[index], ctx);
    if (result != 0)
      return result;
  }
  return 0;
}

// -----[ list_dup ]-------------------------------------------------
gds_list_t * list_dup(gds_list_t * list, gds_list_dup_f dup)
{
  unsigned int index;
  gds_list_t * new_list= list_create(list->ops.cmp,
				     list->ops.destroy,
				     list->resize_step);
  new_list->size= list->size;
  new_list->length= list->length;
	
  new_list->items= malloc(sizeof(void *)*new_list->size);
  assert(new_list->items != NULL);
  if (dup == NULL) {
    memcpy(new_list->items, list->items,
	   sizeof(void *)*new_list->length);
  } else {
    for (index= 0; index < new_list->length; index++)
      new_list->items[index]= dup(list->items[index]);
  }
  return new_list;
}

