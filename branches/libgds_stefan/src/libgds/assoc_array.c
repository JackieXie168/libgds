// ==================================================================
// @(#)assoc_array.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 05/01/2007
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <libgds/assoc_array.h>
#include <libgds/memory.h>
#include <libgds/str_util.h>

typedef struct {
  char * key;
  void * value;
} _assoc_item_t;

// -----[ _assoc_item_create ]---------------------------------------
static inline _assoc_item_t * _assoc_item_create(const char * key,
						 const void * value)
{
  _assoc_item_t * item= (_assoc_item_t *) MALLOC(sizeof(_assoc_item_t));
  item->key= str_create(key);
  item->value= (void *) value;
  return item;
}

// -----[ _assoc_item_destroy ]--------------------------------------
static inline void _assoc_item_destroy(_assoc_item_t ** item_ref,
				       const assoc_array_destroy_f destroy)
{
  if (*item_ref != NULL) {
    str_destroy(&(*item_ref)->key);
    if (destroy != NULL)
      destroy((*item_ref)->value);
    FREE(*item_ref);
    *item_ref= NULL;
  }
}

// -----[ _assoc_array_item_compare ]--------------------------------
/**
 * Private helper function used to compare 2 keys.
 */
static int _assoc_array_item_compare(const void * item1,
				     const void * item2,
				     unsigned int elt_size)
{
  _assoc_item_t * assoc_item1= *((_assoc_item_t **) item1);
  _assoc_item_t * assoc_item2= *((_assoc_item_t **) item2);

  return strcmp(assoc_item1->key, assoc_item2->key);
}

// -----[ _assoc_array_item_destroy ]--------------------------------
/**
 * Private helper function used to destroy each item.
 */
static void _assoc_array_item_destroy(void * item,
				      const void * ctx)
{
  _assoc_item_destroy((_assoc_item_t **) item,
		      (assoc_array_destroy_f) ctx);
}

// -----[ assoc_array_create ]---------------------------------------
gds_assoc_array_t * assoc_array_create(assoc_array_destroy_f destroy)
{
  return (gds_assoc_array_t *) ptr_array_create(ARRAY_OPTION_SORTED,
						_assoc_array_item_compare,
						_assoc_array_item_destroy,
						destroy);
}

// -----[ assoc_array_destroy ]--------------------------------------
void assoc_array_destroy(gds_assoc_array_t ** array_ref)
{
  ptr_array_destroy(array_ref);
}

// -----[ assoc_array_length ]-------------------------------------
unsigned int assoc_array_length(gds_assoc_array_t * array)
{
  return ptr_array_length(array);
}

// -----[ assoc_array_exists ]---------------------------------------
int assoc_array_exists(gds_assoc_array_t * array, const char * key)
{
  _assoc_item_t tmp;
  _assoc_item_t * tmp_ref= &tmp;
  unsigned int index;

  tmp.key= (char *) key;
  if (!ptr_array_sorted_find_index(array, &tmp_ref, &index))
    return 1;
  return 0;
}

// -----[ assoc_array_get ]------------------------------------------
void * assoc_array_get(gds_assoc_array_t * array, const char * key)
{
  _assoc_item_t tmp;
  _assoc_item_t * tmp_ref= &tmp;
  unsigned int index;

  tmp.key= (char *) key;
  if (!ptr_array_sorted_find_index(array, &tmp_ref, &index))
    return ((_assoc_item_t *) array->data[index])->value;
  return NULL;
}

// -----[ assoc_array_set ]------------------------------------------
/**
 * RETURNS:
 *    0 in case of success
 *   -1 in case of failure
 */
int assoc_array_set(gds_assoc_array_t * array, const char * key,
		    const void * value)
{
  _assoc_item_t * item= _assoc_item_create(key, value);
  if (ptr_array_add(array, &item) < 0) {
    return -1;
  }
  return 0;
}

// -----[ assoc_array_for_each ]-------------------------------------
int assoc_array_for_each(gds_assoc_array_t * array,
			 assoc_array_foreach_f foreach,
			 void * ctx)
{
  unsigned int index;
  _assoc_item_t * item;
  int result;

  for (index= 0; index < ptr_array_length(array); index++) {
    item= (_assoc_item_t *) array->data[index];
    result= foreach(item->key, item->value, ctx);
    if (result)
      return result;
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////
//
// ENUMERATOR
//
/////////////////////////////////////////////////////////////////////

typedef struct {
  unsigned int         index;
  gds_assoc_array_t  * array;
  int                  key_or_value; // key=0 / value=1
} _enum_ctx_t;

// -----[ _enum_has_next ]-------------------------------------------
static int _enum_has_next(void * ctx)
{
  _enum_ctx_t * enum_ctx= (_enum_ctx_t *) ctx;
  return (enum_ctx->index  < _array_length((array_t *) enum_ctx->array));
}

// -----[ _enum_get_next ]-------------------------------------------
static void * _enum_get_next(void * ctx)
{
  _enum_ctx_t * enum_ctx= (_enum_ctx_t *) ctx;
  _assoc_item_t * item;
  
  _array_get_at((array_t *) enum_ctx->array, enum_ctx->index, &item);
  enum_ctx->index++;
  switch (enum_ctx->key_or_value) {
  case ASSOC_ARRAY_ENUM_KEYS:
    return item->key;
  case ASSOC_ARRAY_ENUM_VALUES:
    return item->value;
  default:
    abort();
  }
}

// -----[ _enum_destroy ]--------------------------------------------
static void _enum_destroy(void * ctx)
{
  _enum_ctx_t * enum_ctx= (_enum_ctx_t *) ctx;
  FREE(enum_ctx);
}

// -----[ assoc_array_get_enum ]-------------------------------------
gds_enum_t * assoc_array_get_enum(gds_assoc_array_t * array,
				  int key_or_value)
{
  _enum_ctx_t * ctx=
    (_enum_ctx_t *) MALLOC(sizeof(_enum_ctx_t));
  ctx->array= array;
  ctx->index= 0;
  ctx->key_or_value= key_or_value;
  return enum_create(ctx,
		     _enum_has_next,
		     _enum_get_next,
		     _enum_destroy);
}
