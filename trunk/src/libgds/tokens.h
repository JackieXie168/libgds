// =================================================================
// @(#)tokens.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 20/08/2003
// $Id$
// =================================================================

#ifndef __GDS_TOKENS_H__
#define __GDS_TOKENS_H__

#include <libgds/array.h>
#include <libgds/types.h>

typedef SPtrArray gds_tokens_t;

#ifdef __cplusplus
extern "C" {
#endif

  // ----- tokens_create --------------------------------------------
  gds_tokens_t * tokens_create();
  // ----- tokens_destroy -------------------------------------------
  void tokens_destroy(gds_tokens_t ** tokens_ref);
  // ----- tokens_add -----------------------------------------------
  int tokens_add(const gds_tokens_t * tokens, char * token);
  // ----- tokens_add_copy ------------------------------------------
  int tokens_add_copy(const gds_tokens_t * tokens, char * token);
  // ----- tokens_get_num -------------------------------------------
  unsigned int tokens_get_num(const gds_tokens_t * tokens);
  // ----- tokens_get_at --------------------------------------------
  char * tokens_get_string_at(const gds_tokens_t * tokens,
			      unsigned int index);
  // ----- tokens_get_long_at ---------------------------------------
  int tokens_get_long_at(const gds_tokens_t * tokens, unsigned int index,
			 long int * value);
  // ----- tokens_get_int_at ----------------------------------------
  int tokens_get_int_at(const gds_tokens_t * tokens, unsigned int index,
			int * value);
  // ----- tokens_get_ulong_at --------------------------------------
  int tokens_get_ulong_at(const gds_tokens_t * tokens, unsigned int index,
			  unsigned long int * value);
  // ----- tokens_get_uint_at ---------------------------------------
  int tokens_get_uint_at(const gds_tokens_t * tokens, unsigned int index,
			 unsigned int * value);
  // ----- tokens_get_double_at -------------------------------------
  int tokens_get_double_at(const gds_tokens_t * tokens, unsigned int index,
			   double * value);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_TOKENS_H__ */
