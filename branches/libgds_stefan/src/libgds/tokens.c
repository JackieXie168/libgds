// =================================================================
// @(#)tokens.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 20/08/2003
// $Id$
// =================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <str_util.h>

#include <libgds/memory.h>
#include <libgds/tokens.h>

// -----[ _tokens_item_destroy ]-------------------------------------
static void _tokens_item_destroy(void * item, const void * ctx)
{
  str_destroy((char **) item);
}

// -----[ tokens_create ]--------------------------------------------
gds_tokens_t * tokens_create()
{
  gds_tokens_t * tks= (gds_tokens_t *) MALLOC(sizeof(gds_tokens_t));
  tks->num_tokens= 0;
  tks->array= ptr_array_create(0, NULL, _tokens_item_destroy, NULL);
  return tks;
}

// -----[ tokens_destroy ]-------------------------------------------
void tokens_destroy(gds_tokens_t ** tks_ref)
{
  gds_tokens_t * tks= *tks_ref;
  if (tks != NULL) {
    ptr_array_destroy(&tks->array);
    FREE(tks);
    *tks_ref= NULL;
  }
}

// -----[ tokens_clear ]---------------------------------------------
void tokens_clear(gds_tokens_t * tks)
{
  tks->num_tokens= 0;
}

// -----[ tokens_add ]-----------------------------------------------
int tokens_add(gds_tokens_t * tks, char * token)
{
  int result;

  // No need to extend array length, insert at end
  if (tks->num_tokens < ptr_array_length(tks->array)) {
    FREE(tks->array->data[tks->num_tokens]);
    tks->array->data[tks->num_tokens]= token;
    result= tks->num_tokens;
  } else {
    result= ptr_array_add(tks->array, &token);
  }
  if (result < 0)
    return -1;
  tks->num_tokens++;
  return result;
}

// -----[ tokens_add_copy ]------------------------------------------
/**
 *
 */
int tokens_add_copy(gds_tokens_t * tks, const char * token)
{
  return tokens_add(tks, str_create(token));
}

// -----[ tokens_get_num ]-------------------------------------------
unsigned int tokens_get_num(const gds_tokens_t * tks)
{
  return tks->num_tokens;
}

// -----[ tokens_get_string_at ]-------------------------------------
const char * tokens_get_string_at(const gds_tokens_t * tks,
				  unsigned int index)
{
  if (index >= tokens_get_num(tks))
    return NULL;
  return (char *) tks->array->data[index];
}

// -----[ tokens_get_long_at ]---------------------------------------
int tokens_get_long_at(const gds_tokens_t * tks, unsigned int index,
		       long int * value)
{
  if (index >= tokens_get_num(tks))
    return -1;
  if (str_as_long(tks->array->data[index], value) < 0)
    return -1;
  return 0;
}

// -----[ tokens_get_int_at ]----------------------------------------
int tokens_get_int_at(const gds_tokens_t * tks, unsigned int index,
		      int * value)
{
  if (index >= tokens_get_num(tks))
    return -1;
  if (str_as_int(tks->array->data[index], value) < 0)
    return -1;
  return 0;
}

// -----[ tokens_get_ulong_at ]--------------------------------------
int tokens_get_ulong_at(const gds_tokens_t * tks, unsigned int index,
			unsigned long int * value)
{
  if (index >= tokens_get_num(tks))
    return -1;
  if (str_as_ulong(tks->array->data[index], value) < 0)
    return -1;
  return 0;
}

// -----[ tokens_get_uint_at ]---------------------------------------
int tokens_get_uint_at(const gds_tokens_t * tks, unsigned int index,
		       unsigned int * value)
{
  if (index >= tokens_get_num(tks))
    return -1;
  if (str_as_uint(tks->array->data[index], value) < 0)
    return -1;
  return 0;
}

// -----[ tokens_get_double_at ---------------------------------------
int tokens_get_double_at(const gds_tokens_t * tks, unsigned int index,
			 double * value)
{
  if (index >= tokens_get_num(tks))
    return -1;
  if (str_as_double(tks->array->data[index], value) < 0)
    return -1;
  return 0;
}
