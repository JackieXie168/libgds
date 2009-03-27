// ==================================================================
// @(#)sequence.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 23/11/2002
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgds/memory.h>
#include <libgds/sequence.h>

// -----[ sequence_create ]------------------------------------------
gds_seq_t * sequence_create(gds_seq_cmp_f cmp,
			    gds_seq_destroy_f destroy)
{
  gds_seq_t * sequence= (gds_seq_t *) MALLOC(sizeof(gds_seq_t));
  sequence->size= 0;
  sequence->cmp= cmp;
  sequence->destroy= destroy;
  sequence->items= NULL;
  return sequence;
}

// -----[ sequence_destroy ]-----------------------------------------
void sequence_destroy(gds_seq_t ** seq_ref)
{
  unsigned int index;
  gds_seq_t * seq= *seq_ref;

  if (seq != NULL) {
    if (seq->items != NULL) {
      if (seq->destroy != NULL)
	for (index= 0; index < seq->size; index++)
	  seq->destroy(&seq->items[index]);
      FREE(seq->items);
      seq->items= NULL;
    }
    FREE(seq);
    *seq_ref= NULL;
  }
}

// -----[ sequence_index_of ]----------------------------------------
/**
 * ITEM found => return -1INDEX of item (>= 0)
 * ITEM not found => -1
 */
int sequence_index_of(gds_seq_t * seq, void * item,
		      unsigned int * index_ref)
{
  unsigned int index= 0;

  while (index < seq->size) {
    if (((seq->cmp != NULL) &&
	 (seq->cmp(seq->items[index], item))) ||
	(seq->items[index] == item)) {
      *index_ref= index;
      return 0;
    }
    index++;
  }
  return -1;
}

// ----- sequence_insert_at -----------------------------------------
void sequence_insert_at(gds_seq_t * seq, unsigned int index,
			void * item)
{
  assert(index <= seq->size);
  seq->size++;
  if (seq->items != NULL) {
    seq->items= REALLOC(seq->items,
			sizeof(void *)*seq->size);
    memmove(&seq->items[index+1], &seq->items[index],
	    sizeof(void *)*(seq->size-index-1));
  } else {
    seq->items= MALLOC(sizeof(void *)*seq->size);
  }
  seq->items[index]= item;
}

// ----- sequence_add -----------------------------------------------
void sequence_add(gds_seq_t * seq, void * item)
{
  sequence_insert_at(seq, seq->size, item);
}

// ----- sequence_remove --------------------------------------------
int sequence_remove(gds_seq_t * seq, void * item)
{
  unsigned int index;
  if (sequence_index_of(seq, item, &index) < 0)
    return -1;
  sequence_remove_at(seq, index);
  return 0;
}

// ----- sequence_remove_at -----------------------------------------
void sequence_remove_at(gds_seq_t * seq, unsigned int index)
{
  assert(index < seq->size);

  if (seq->size-index > 0)
    memmove(&seq->items[index], &seq->items[index+1],
	    seq->size-index-1);
  seq->size--;
  if (seq->size == 0) {
    FREE(seq->items);
    seq->items= NULL;
  } else {
    seq->items= (void *) REALLOC(seq->items, seq->size*sizeof(void *));
  }
}

// ----- sequence_for_each ------------------------------------------
int sequence_for_each(gds_seq_t * seq, gds_seq_foreach_f foreach,
		       void * ctx)
{
  unsigned int index;
  int result;

  for (index= 0; index < seq->size; index++) {
    result= foreach(seq->items[index], ctx);
    if (result < 0)
      return result;
  }
  return 0;
}

// ----- sequence_copy ----------------------------------------------
gds_seq_t * sequence_copy(gds_seq_t * seq, gds_seq_copy_f copy)
{
  unsigned int index;

  gds_seq_t * new_seq= sequence_create(seq->cmp, seq->destroy);
  new_seq->size= seq->size;
  if (new_seq->size > 0) {
    new_seq->items= MALLOC(sizeof(void *)*new_seq->size);
    if (copy == NULL)
      memcpy(new_seq->items, seq->items, sizeof(void *)*new_seq->size);
    else
      for (index= 0; index < new_seq->size; index++)
	new_seq->items[index]= copy(seq->items[index]);
  }
  return new_seq;
}
