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
#include <tokens.h>

// -----[ _tokens_item_destroy ]-------------------------------------
static void _tokens_item_destroy(void * item, const void * ctx)
{
  str_destroy((char **) item);
}

// ----- tokens_create ----------------------------------------------
/**
 *
 */
gds_tokens_t * tokens_create()
{
  return (gds_tokens_t *) ptr_array_create(0, NULL,
					   _tokens_item_destroy, NULL);
}

// ----- tokens_destroy ---------------------------------------------
/**
 *
 */
void tokens_destroy(gds_tokens_t ** tokens_ref)
{
  ptr_array_destroy((SPtrArray **) tokens_ref);
}

// ----- tokens_add -------------------------------------------------
/**
 *
 */
int tokens_add(const gds_tokens_t * tokens, char * token)
{
  return ptr_array_add((SPtrArray *) tokens, &token);
}

// ----- tokens_add_copy --------------------------------------------
/**
 *
 */
int tokens_add_copy(const gds_tokens_t * tokens, char * token)
{
  char * token_copy= str_create(token);
  return ptr_array_add((SPtrArray *) tokens, &token_copy);
}

// ----- tokens_get_num ---------------------------------------------
/**
 *
 */
unsigned int tokens_get_num(const gds_tokens_t * tokens)
{
  return ptr_array_length((SPtrArray *) tokens);
}

// ----- tokens_get_string_at ---------------------------------------
/**
 *
 */
char * tokens_get_string_at(const gds_tokens_t * tokens, unsigned int index)
{
  if (index < tokens_get_num(tokens))
    return (char *) tokens->data[index];
  return NULL;
}

// ----- tokens_get_long_at -----------------------------------------
/**
 *
 */
int tokens_get_long_at(const gds_tokens_t * tokens, unsigned int index,
		       long int * value)
{
  if (index >= tokens_get_num(tokens))
    return -1;
  if (str_as_long(tokens->data[index], value) < 0)
    return -1;
  return 0;
}

// ----- tokens_get_int_at ------------------------------------------
/**
 *
 */
int tokens_get_int_at(const gds_tokens_t * tokens, unsigned int index,
		      int * value)
{
  if (index >= tokens_get_num(tokens))
    return -1;
  if (str_as_int(tokens->data[index], value) < 0)
    return -1;
  return 0;
}

// ----- tokens_get_ulong_at ----------------------------------------
/**
 * The function uses 'strtoll()' in order to convert the string to a
 * 'long long int'.
 *
 * Note: we cannot use 'strtol()' since it is limited to a maximum
 * value of LONG_MAX while we need ULONG_MAX. We cannot use
 * 'strtoul()' since it will convert a negative number to a positive
 * number.
 */
int tokens_get_ulong_at(const gds_tokens_t * tokens, unsigned int index,
			unsigned long int * value)
{
  if (index >= tokens_get_num(tokens))
    return -1;
  if (str_as_ulong(tokens->data[index], value) < 0)
    return -1;
  return 0;
}

// ----- tokens_get_uint_at -----------------------------------------
/**
 *
 */
int tokens_get_uint_at(const gds_tokens_t * tokens, unsigned int index,
		       unsigned int * value)
{
  if (index >= tokens_get_num(tokens))
    return -1;
  if (str_as_uint(tokens->data[index], value) < 0)
    return -1;
  return 0;
}

// ----- tokens_get_double_at ---------------------------------------
/**
 *
 */
int tokens_get_double_at(const gds_tokens_t * tokens, unsigned int index,
			 double * value)
{
  if (index >= tokens_get_num(tokens))
    return -1;
  if (str_as_double(tokens->data[index], value) < 0)
    return -1;
  return 0;
}
