// ==================================================================
// @(#)hash.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 03/12/2004
// $Id$
// ==================================================================

/**
 * This code implements a hash table structure. The hash table is
 * implemented using a dynamic array. Each array cell can contain a
 * sorted list of elements to manage collisions.
 *
 * The same element can be inserted as many times as needed. Each
 * element is associated with a reference counter. An element is
 * freed as soon as its reference counter drops to 0.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <libgds/array.h>
#include <libgds/memory.h>
#include <libgds/stream.h>
#include <libgds/hash.h>

typedef struct {
  gds_hash_cmp_f     elt_cmp;
  gds_hash_destroy_f elt_destroy;
  gds_hash_compute_f hash_compute;
} hash_ops_t;

struct gds_hash_set_t {
  unsigned int    size;
  unsigned int    num_elts;
  float           resize_thr;
  hash_ops_t      ops;
  ptr_array_t  ** items;
};

typedef struct {
  hash_ops_t   * ops;
  void         * item;
  unsigned int   refcnt;
  uint32_t       cur_key;
} hash_elt_t;

// ----- _hash_element_compare --------------------------------------
/**
 *
 */
static int _hash_element_compare(const void * elt1,
				 const void * elt2,
				 unsigned int elt_size)
{
  hash_elt_t * hash_elt1= *((hash_elt_t **) elt1);
  hash_elt_t * hash_elt2= *((hash_elt_t **) elt2);

  return hash_elt1->ops->elt_cmp(hash_elt1->item,
				 hash_elt2->item,
				 elt_size);
}

// ----- _hash_element_remove ---------------------------------------
/**
 *
 */
static void _hash_element_remove(void * elt, const void * ctx)
{
  hash_elt_t * hash_elt= *((hash_elt_t **) elt);

  if (hash_elt != NULL)
    FREE(hash_elt);
}

// ----- _hash_element_destroy --------------------------------------
/**
 *
 */
static void _hash_element_destroy(void * elt, const void * ctx)
{
  hash_elt_t * hash_elt= *((hash_elt_t **) elt);

  if (hash_elt->ops->elt_destroy != NULL)
    hash_elt->ops->elt_destroy(hash_elt->item);
  FREE(hash_elt);
}

// ----- _hash_element_unref ----------------------------------------
/**
 * Decrease the reference count for the given hash element.
 *
 * Return value:
 *   new reference count
 */
static inline int _hash_element_unref(hash_elt_t * elt)
{
  elt->refcnt--;
  return elt->refcnt;
}

// ----- _hash_element_ref ------------------------------------------
/**
 * Increase the reference count for the given hash element.
 *
 * Return value:
 *   new reference count
 */
static inline int _hash_element_ref(hash_elt_t * elt)
{
  elt->refcnt++;
  return elt->refcnt;
}

// ----- _hash_element_init -----------------------------------------
/**
 *
 */
static hash_elt_t * _hash_element_init(void * item,
				       hash_ops_t * ops)
{
  hash_elt_t * elt= MALLOC(sizeof(hash_elt_t));
  elt->ops= ops;
  elt->item= item;
  elt->refcnt= 0;
  return elt;
}

// ----- _hash_element_search ---------------------------------------
/**
 * Lookup an element in a hash table bucket (ptr-array).
 *
 * Return value:
 *   -1 if element could not be found.
 *   0  if element was found. In this case, *puIndex will contain the
 *      index of the element.
 */
static inline int _hash_element_search(const ptr_array_t * aHashElts,
				       void * item, 
				       unsigned int * index_ref)
{
  hash_elt_t elt;
  hash_elt_t * elt_ref= &elt;
  
  elt.item= item;
  return ptr_array_sorted_find_index(aHashElts, &elt_ref, index_ref);
}

// -----[ _hash_element_set_current_key ]----------------------------
/**
 * What is the purpose of this function ?
 */
static inline void _hash_element_set_current_key(hash_elt_t * elt,
						 const uint32_t key)
{
  elt->cur_key= key;
}

// -----[ _hash_element_get_current_key ]----------------------------
/**
 * What is the purpose of this function ?
 */
static inline uint32_t _hash_element_get_current_key(const hash_elt_t * elt)
{
  return elt->cur_key;
}

// ----- _hash_element_add ------------------------------------------
/**
 * Add an element to an hash table bucket (ptr-array).
 */
static inline hash_elt_t * _hash_element_add(const ptr_array_t * aHashElt,
					   void * item,
					   hash_ops_t * ops)
{
  hash_elt_t * elt= _hash_element_init(item, ops);
  assert(ptr_array_add(aHashElt, &elt) >= 0);
  return elt;
}

// -----[ hash_set_create ]------------------------------------------
gds_hash_set_t * hash_set_create(unsigned int size,
				 float resize_thr, 
				 gds_hash_cmp_f cmp,
				 gds_hash_destroy_f destroy,
				 gds_hash_compute_f compute)
{
  gds_hash_set_t * hash= MALLOC(sizeof(gds_hash_set_t));

  hash->items= MALLOC(sizeof(void *)*size);
  memset(hash->items, 0, sizeof(void *)*size);

  assert(compute != NULL);
  assert(resize_thr >= 0.0);
  assert(resize_thr < 1.0);

  hash->ops.elt_cmp= cmp;
  hash->ops.elt_destroy= destroy;
  hash->ops.hash_compute= compute;

  hash->size= size;
  hash->resize_thr= resize_thr;
  hash->num_elts= 0;

  return hash;
}

// -----[ hash_set_destroy ]-----------------------------------------
void hash_set_destroy(gds_hash_set_t ** hash_ref)
{
  unsigned int index;

  if (*hash_ref != NULL) {
    for (index= 0; index < (*hash_ref)->size; index++) {
      if ((*hash_ref)->items[index] != NULL)
	ptr_array_destroy(&((*hash_ref)->items[index]));
    }
    FREE((*hash_ref)->items);
    FREE((*hash_ref) );
    (*hash_ref)= NULL;
  } 
}

// -----[ _hash_set_get_hash_array ]---------------------------------
/**
 * Initialize a new hash table bucket (ptr-array).
 */
static inline
ptr_array_t * _hash_set_get_hash_array(gds_hash_set_t * hash,
				       uint32_t key)
{
  assert(key < hash->size);

  if (hash->items[key] == NULL) {
    hash->items[key]=
      ptr_array_create(ARRAY_OPTION_UNIQUE|ARRAY_OPTION_SORTED,
		       _hash_element_compare,
		       _hash_element_destroy, NULL);
  }
  return hash->items[key];
}

// -----[ _hash_set_compute_key ]------------------------------------
/**
 * Wrapper for computing the hash key for an element.
 */
static inline
uint32_t _hash_set_compute_key(const gds_hash_set_t * hash, const void * item)
{
  return hash->ops.hash_compute(item, hash->size);
}

// -----[ hash_set_rehash ]------------------------------------------
/**
 * Re-hash the entire hash table.
 */
static void _hash_set_rehash(gds_hash_set_t * hash)
{
  uint32_t key;
  unsigned int index, index2;
  unsigned int new_size;
  unsigned int old_size;
  ptr_array_t * aHashElt;
  ptr_array_t * aHashEltNew;
  hash_elt_t * hash_elt;
  void * item= NULL;
  
  old_size= hash->size;
  new_size= hash->size*2;

  hash->items= REALLOC(hash->items, sizeof(uint32_t)*new_size);
  memset((hash->items)+old_size, 0, sizeof(uint32_t)*hash->size);
  hash->size= new_size;

  for (index = 0; index < old_size; index++) {
    aHashElt= hash->items[index];
    if (aHashElt != NULL) {
      for (index2= 0; index2 < ptr_array_length(aHashElt); index2++) {
	// Extract the Elt
	hash_elt= aHashElt->data[index2];
	key= _hash_set_compute_key(hash, item);
	/* If the old key is the same as the new. Don't move the hash_elt as it
	 * stays in the same array of the hash table */
	if (key != _hash_element_get_current_key(hash_elt)) {
	  item= hash_elt->item;
	  // Remove the element
	  /* change the destroy function in the array structure as we don't
	   * want the pElt to be destroyed */
	  ptr_array_set_fdestroy(aHashElt, _hash_element_remove, NULL);
	  ptr_array_remove_at(aHashElt, index2);
	  // Get his new aHashElt array in which setting pElt
	  aHashEltNew= _hash_set_get_hash_array(hash, key);
	  hash_elt= _hash_element_add(aHashEltNew, item, &hash->ops);
	  /* Change the current key of the Elt */
	  _hash_element_set_current_key(hash_elt, key);
	  /* restore the destroy function */
	  ptr_array_set_fdestroy(aHashEltNew, _hash_element_destroy, NULL);
	} 
      } 
    }
  }
}


// -----[ hash_set_add ]---------------------------------------------
void * hash_set_add(gds_hash_set_t * hash, void * item)
{
  unsigned int index= 0;
  hash_elt_t * elt;
  ptr_array_t * aHashElt;
  uint32_t key;

  key= _hash_set_compute_key(hash, item);

  // Return the hash bucket for this key (create the bucket if required)
  aHashElt= _hash_set_get_hash_array(hash, key);

  // Lookup in the bucket for an existing element
  if (_hash_element_search(aHashElt, item, &index) == -1) {

    // Element does not exist yet

    // ----- re-hashing ? -------------------------------------------
    // If the hash occupancy threshold is higher than configured,
    // increase the hash table size and re-hash every element.
    // Note: this is disabled if threshold == 0
    if (hash->resize_thr > 0.0) {
      if (++hash->num_elts >
	  (uint32_t)((float)hash->size*hash->resize_thr)) {
	_hash_set_rehash(hash);
	// Recompute the new element's key and find the new bucket as
	// the hash table size has changed.
	key= _hash_set_compute_key(hash, item);
	aHashElt= _hash_set_get_hash_array(hash, key);
      }
    }

    elt= _hash_element_add(aHashElt, item, &hash->ops);
    _hash_element_set_current_key(elt, key);

  } else {

    // Element already exists
    elt= aHashElt->data[index];

  }

  // Increase reference count for existing / new element
  _hash_element_ref(elt);

  return elt->item;
}

// -----[ hash_set_search ]------------------------------------------
/**
 * Lookup an element equivalent to pElt.
 *
 * Return value:
 *   NULL if no elt found
 *   pointer to found elt otherwise
 */
void * hash_set_search(const gds_hash_set_t * hash, void * item)
{
  unsigned int index;
  ptr_array_t * aHashElts;
  hash_elt_t * searched_elt = NULL;
  uint32_t key= hash->ops.hash_compute(item, hash->size);

  aHashElts= hash->items[key];
  if (aHashElts != NULL) {
    if (_hash_element_search(aHashElts, item, &index) != -1)
      searched_elt= (hash_elt_t *) aHashElts->data[index];
  }
  return (searched_elt == NULL) ? NULL : searched_elt->item;
}

// -----[ hash_set_remove ]------------------------------------------
int hash_set_remove(gds_hash_set_t * hash, void * item)
{
  uint32_t key= hash->ops.hash_compute(item, hash->size);
  ptr_array_t * aHashElt;
  int result= HASH_ERROR_NO_MATCH;
  unsigned int index;
  
  aHashElt= hash->items[key];
  if (aHashElt != NULL) {
    if (_hash_element_search(aHashElt, item, &index) != -1) {
      result= HASH_SUCCESS_UNREF;
      if (_hash_element_unref((hash_elt_t *) aHashElt->data[index]) <= 0){
	result= HASH_SUCCESS;
	hash->num_elts--;
	ptr_array_remove_at(aHashElt, index);
      }
    }
  }
  return result;
}

// -----[ hash_set_get_refcnt ]--------------------------------------
/**
 * Return number of references on the given item.
 *
 * Return value:
 *   0 if the element does not exist
 *   reference counter otherwise
 */
unsigned int hash_set_get_refcnt(const gds_hash_set_t * hash, void * item)
{
  unsigned int index;
  ptr_array_t * hash_items;
  hash_elt_t * searched_elt= NULL;
  uint32_t key;

  key= hash->ops.hash_compute(item, hash->size);
  hash_items= hash->items[key];
  if (hash_items == NULL)
    return 0;

  if (_hash_element_search(hash_items, item, &index) == -1)
    return 0;

  searched_elt= hash_items->data[index];
  return searched_elt->refcnt;
}

// -----[ hash_set_for_each_key ]------------------------------------
/**
 * Call the given callback function foreach key in the hash table. The
 * item which is passed to the callback function is the array at the
 * given key. This makes possible to evaluate the hash function. If
 * the distribution is quite uniform, the hash function is
 * good. Otherwise, it is time to look for another one.
 *
 * Note: the callback can be called with an item which is NULL.
 */
int hash_set_for_each_key(const gds_hash_set_t * hash,
			  gds_hash_foreach_f foreach, 
			  void * ctx)
{
  int result;
  uint32_t key;
  ptr_array_t * pHashItems;

  for (key= 0; key < hash->size; key++) {
    pHashItems= hash->items[key];
    result= foreach(pHashItems, ctx);
    if (result < 0)
      return result;
  }
  return 0;
}

// -----[ hash_set_for_each ]----------------------------------------
/**
 * Call the given callback function foreach item in the hash table.
 */
int hash_set_for_each(const gds_hash_set_t * hash,
		      gds_hash_foreach_f foreach,
		      void * ctx)
{
  int result;
  uint32_t key;
  unsigned int index;
  ptr_array_t * pHashItems;
  void * item;

  for (key= 0; key < hash->size; key++) {
    pHashItems= hash->items[key];
    if (pHashItems != NULL) {
      for (index= 0; index < ptr_array_length(pHashItems); index++) {
	item= ((hash_elt_t *) pHashItems->data[index])->item;
	result= foreach(item, ctx);
	if (result < 0)
	  return result;
      }
    }
  }
  return 0;
}

// -----[ hash_set_dump ]--------------------------------------------
void hash_set_dump(const gds_hash_set_t * hash)
{
  uint32_t key;
  unsigned int index;
  ptr_array_t * pHashItems;
  hash_elt_t * elt;

  fprintf(stderr, "**********************************\n");
  fprintf(stderr, "hash-size: %u\n", hash->size);
  for (key= 0; key < hash->size; key++) {
    pHashItems= hash->items[key];
    if (pHashItems != NULL) {
      fprintf(stderr, "->key:%u (%u)\n", (unsigned int) key, ptr_array_length(pHashItems));
      for (index= 0; index < ptr_array_length(pHashItems); index++) {
	elt= (hash_elt_t *) pHashItems->data[index];
	fprintf(stderr, "  [%u]: (%p) refcnt:%u\n",
		index, elt->item, elt->refcnt);
      }
    }
  }
  fprintf(stderr, "**********************************\n");
}


/////////////////////////////////////////////////////////////////////
//
// ENUMERATION
//
/////////////////////////////////////////////////////////////////////

typedef struct {
  unsigned int     index1;
  unsigned int     index2;
  gds_hash_set_t * hash;
} _enum_ctx_t;

// -----[ _enum_has_next ]-------------------------------------------
int _enum_has_next(void * ctx)
{
  _enum_ctx_t * enum_ctx= (_enum_ctx_t *) ctx;
  ptr_array_t * pHashItems;
  unsigned int index1;


  index1= enum_ctx->index1;
  /* First, have a look at the index we are in the hash table */
  /* If it is not satisfying we *have* to get to the place of the potential
   * next item which can "far away" from the last */

  /* Have we already get all the items? */
  if (index1 >= enum_ctx->hash->size )
    return 0;

  /* is there anything more in the array we are going through? */
  pHashItems= enum_ctx->hash->items[enum_ctx->index1];
  if ((pHashItems != NULL) &&
      (enum_ctx->index2 < ptr_array_length(pHashItems)))
    return 1;

  index1++;
  /* We have to find in the following arrays for the next item */
  for (; index1 < enum_ctx->hash->size; index1++) {
    pHashItems= enum_ctx->hash->items[index1];
    if ((pHashItems != NULL) && (ptr_array_length(pHashItems) > 0)) 
      return 1;
  }
  return 0;
}

// -----[ _enum_get_next ]-------------------------------------------
void * _enum_get_next(void * ctx)
{
  _enum_ctx_t * enum_ctx= (_enum_ctx_t *) ctx;
  ptr_array_t * pHashItems= NULL;
  void * item= NULL;

  /* Have we already get all the items? */
  if (enum_ctx->index1 >= enum_ctx->hash->size)
    return NULL;

  /* is the anything more in the array we are going through? */
  pHashItems= enum_ctx->hash->items[enum_ctx->index1];
  if ((pHashItems != NULL) &&
      (enum_ctx->index2 < ptr_array_length(pHashItems))) {
    item= ((hash_elt_t *) pHashItems->data[enum_ctx->index2])->item;
    enum_ctx->index2++;
    return &item;
  }

  /* We have to return an item from another array of the hash table */
  enum_ctx->index1++;
  /* We may initialize it to 1 as we've got to return the first element anyway */
  enum_ctx->index2= 1;
  for (; enum_ctx->index1 < enum_ctx->hash->size; enum_ctx->index1++) {
    pHashItems= enum_ctx->hash->items[enum_ctx->index1];
    if ((pHashItems != NULL) && (ptr_array_length(pHashItems) > 0))
      return &((hash_elt_t *) pHashItems->data[0])->item;
  }
  return NULL;
}

// -----[ _enum_destroy ]--------------------------------------------
void _enum_destroy(void * ctx)
{
  _enum_ctx_t * enum_ctx= (_enum_ctx_t *) ctx;
  FREE(enum_ctx);
}

// -----[ hash_set_get_enum ]----------------------------------------
gds_enum_t * hash_set_get_enum(gds_hash_set_t * hash)
{
  _enum_ctx_t * ctx=
    (_enum_ctx_t *) MALLOC(sizeof(_enum_ctx_t));
  ctx->index1= 0;
  ctx->index2= 0;
  ctx->hash= hash;
  return enum_create(ctx,
		     _enum_has_next,
		     _enum_get_next,
		     _enum_destroy);
}
