// ==================================================================
// @(#)dllist.c
//
// Doubly-linked lists
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 26/07/2006
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libgds/dllist.h>
#include <libgds/memory.h>

// -----[ _dllist_item_create ]--------------------------------------
static inline
gds_dllist_item_t * _dllist_item_create(void * user_data,
					gds_dllist_item_t * prev,
					gds_dllist_item_t * next)
{
  gds_dllist_item_t * item=
    (gds_dllist_item_t *) MALLOC(sizeof(gds_dllist_item_t));
  item->user_data= user_data;
  item->prev= prev;
  item->next= next;
  return item;
}

// -----[ _dllist_item_destroy ]-------------------------------------
static inline
void _dllist_item_destroy(gds_dllist_item_t ** item_ref,
			  gds_dllist_destroy_f destroy)
{
  gds_dllist_item_t * item= *item_ref;
  if (item == NULL)
    return;
  if (destroy != NULL)
    destroy(item->user_data);
  FREE(item);
  *item_ref= NULL;
}

// -----[ dllist_create ]--------------------------------------------
gds_dllist_t * dllist_create(gds_dllist_destroy_f destroy)
{
  gds_dllist_t * list= (gds_dllist_t *) MALLOC(sizeof(gds_dllist_t));
  list->root= NULL;
  list->destroy= destroy;
  return list;
}

// -----[ dllist_destroy ]-------------------------------------------
void dllist_destroy(gds_dllist_t ** list_ref)
{
  gds_dllist_item_t * item, * tmp;
  gds_dllist_t * list= *list_ref;

  if (list == NULL)
    return;
  item= list->root;
  while (item != NULL) {
    tmp= item;
    item= item->next;
    _dllist_item_destroy(&tmp, list->destroy);
  }
  FREE(list);
  *list_ref= NULL;
}

// -----[ dllist_insert ]--------------------------------------------
/**
 * Return value:
 *   0 success
 *   -1 invalid index
 */
int dllist_insert(gds_dllist_t * list, unsigned int index,
		  void * user_data)
{
  gds_dllist_item_t * item;

  if (index == 0) {
    list->root= _dllist_item_create(user_data, NULL, list->root);
    return 0;
  }

  item= list->root;
  while ((item != NULL) && (index > 1)) {
    item= item->next;
    index--;
  }
  if (item == NULL)
    return -1;
  item->next= _dllist_item_create(user_data, item, item->next);
  return 0;
}

// -----[ dllist_remove ]--------------------------------------------
/**
 * Return value:
 *   0  success
 *   -1 invalid index
 */
int dllist_remove(gds_dllist_t * list, unsigned int index)
{
  gds_dllist_item_t * tmp; // Pointer to item to delete
  gds_dllist_item_t ** item_ref; // Pointer to pointer of item to delete

  // Not even a single item, abort with error
  if (list->root == NULL)
    return -1;

  item_ref= &(list->root);
  while ((index > 0) && (*item_ref != NULL)) {
    // Note: advance pointer to pointer
    item_ref= &(*item_ref)->next;
    index--;
  }
  if (*item_ref == NULL)
    return -1;

  // Keep reference to current item
  tmp= *item_ref;

  // Update cross-references
  if ((*item_ref)->next != NULL)
    (*item_ref)->next->prev= (*item_ref)->prev;
  *item_ref= (*item_ref)->next;

  // Destroy temporary item
  _dllist_item_destroy(&tmp, list->destroy);
  return 0;
}

// -----[ dllist_append ]--------------------------------------------
void dllist_append(gds_dllist_t * list, void * user_data)
{
  gds_dllist_item_t * item;

  if (list->root == NULL) {
    list->root= _dllist_item_create(user_data, NULL, NULL);
    return;
  }

  item= list->root;
  while (item->next != NULL)
    item= item->next;
  item->next= _dllist_item_create(user_data, item, NULL);
}

// -----[ dllist_get ]-----------------------------------------------
int dllist_get(gds_dllist_t * list, unsigned int index,
	       void ** user_data_ref)
{
  gds_dllist_item_t * item= list->root;

  while ((index > 0) && (item != NULL)) {
    item= item->next;
    index--;
  }
  if (item == NULL)
    return -1;
  *user_data_ref= item->user_data;
  return 0;
}

// -----[ dllist_size ]-----------------------------------------------
unsigned int dllist_size(gds_dllist_t * list)
{
  gds_dllist_item_t * item= list->root;
  unsigned int size= 0;

  while (item != NULL) {
    item= item->next;
    size++;
  }
  return size;
}

// -----[ dllist_for_each ]------------------------------------------
int dllist_for_each(gds_dllist_t * list, void * ctx,
		    gds_dllist_foreach_f foreach)
{
  gds_dllist_item_t * item= list->root;
  int result;

  while (item != NULL) {
    result= foreach(item->user_data, ctx);
    if (result != 0)
      return result;
    item= item->next;
  }
  return 0;
}
