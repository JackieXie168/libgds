// =================================================================
// @(#)tokens.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 20/08/2003
// $Id$
// =================================================================

/**
 * \file
 * Provide a data structure and functions to manage a list of tokens.
 * Tokens are basically just strings, but they could be associated
 * with additional information from the lexer in the future.
 *
 * The main use of the tokens data structure is libGDS's tokenizer.
 */

#ifndef __GDS_TOKENS_H__
#define __GDS_TOKENS_H__

#include <libgds/array.h>
#include <libgds/types.h>

/**
 * Tokens data structure.
 *
 * The data tokens data structure contains an array of tokens that
 * grows monotinically. In addition, the tokens data structure
 * contains a field that tracks the actual number of tokens present
 * in the list.
 *
 * If the same tokens list must be re-used multiple times, there is
 * no need to keep allocating/de-allocating the tokens. Just create
 * the tokens list and clear it before re-useing it.
 */
typedef struct gds_tokens_t {
  /** \internal Number of tokens currently present. */
  unsigned int   num_tokens;
  /** \internal Array of tokens. */
  ptr_array_t  * array;
} gds_tokens_t;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ tokens_create ]------------------------------------------
  /**
   * Create a list of tokens.
   */
  gds_tokens_t * tokens_create();

  // -----[ tokens_destroy ]-----------------------------------------
  /**
   * Destroy a list of tokens.
   *
   * \param tks_ref is a pointer to the tokens to be destroyed.
   */
  void tokens_destroy(gds_tokens_t ** tks_ref);

  // -----[ tokens_clear ]-------------------------------------------
  /**
   * Clear the list of tokens.
   *
   * \param tks is the list of tokens.
   */
  void tokens_clear(gds_tokens_t * tks);

  // -----[ tokens_add ]---------------------------------------------
  /**
   * Add a token to the list of tokens.
   *
   * \param tks   is the list of tokens.
   * \param token is the new token to be added. The token will now be
   *   owned by the tokens list and should not be modified or freed.
   * \retval the insertion index in case of success,
   *   or <0 in case of failure.
   */
  int tokens_add(gds_tokens_t * tks, char * token);

  // -----[ tokens_add_copy ]----------------------------------------
  /**
   * Add a copy of a token to the list of tokens.
   *
   * \param tks   is the list of tokens.
   * \param token is the new token to be added.
   * \retval the insertion index in case of success,
   *   or <0 in case of failure.
   */
  int tokens_add_copy(gds_tokens_t * tks, const char * token);

  // -----[ tokens_get_num ]-----------------------------------------
  /**
   * Get the number of tokens in the list.
   *
   * \param tks is the list of tokens.
   * \retval the number of tokens.
   */
  unsigned int tokens_get_num(const gds_tokens_t * tks);

  // -----[ tokens_get_string_at ]-----------------------------------
  /**
   * Get a token from the list.
   *
   * \param tks   is the list of tokens.
   * \param index is the position of the requested token.
   * \retval the requested token,
   *   or NULL if the index is invalid.
   */
  const char * tokens_get_string_at(const gds_tokens_t * tks,
				    unsigned int index);

  // -----[ tokens_get_long_at ]-------------------------------------
  /**
   * Get a long integer token from the list.
   *
   * \param tks   is the list of tokens.
   * \param index is the position of the requested token.
   * \param value is a pointer to the token value.
   * \retval 0 in case the conversion was successful,
   *   <0 in case the token index is invalid or the conversion failed.
   */
  int tokens_get_long_at(const gds_tokens_t * tks, unsigned int index,
			 long int * value);

  // -----[ tokens_get_int_at ]--------------------------------------
  /**
   * Get an integer token from the list.
   *
   * \param tks   is the list of tokens.
   * \param index is the position of the requested token.
   * \param value is a pointer to the token value.
   * \retval 0 in case the conversion was successful,
   *   <0 in case the token index is invalid or the conversion failed.
   */
  int tokens_get_int_at(const gds_tokens_t * tks, unsigned int index,
			int * value);

  // -----[ tokens_get_ulong_at ]------------------------------------
  /**
   * Get an unsigned long integer token from the list.
   *
   * \param tks   is the list of tokens.
   * \param index is the position of the requested token.
   * \param value is a pointer to the token value.
   * \retval 0 in case the conversion was successful,
   *   <0 in case the token index is invalid or the conversion failed.
   */
  int tokens_get_ulong_at(const gds_tokens_t * tks, unsigned int index,
			  unsigned long int * value);

  // -----[ tokens_get_uint_at ]-------------------------------------
  /**
   * Get an unsigned integer token from the list.
   *
   * \param tks   is the list of tokens.
   * \param index is the position of the requested token.
   * \param value is a pointer to the token value.
   * \retval 0 in case the conversion was successful,
   *   <0 in case the token index is invalid or the conversion failed.
   */
  int tokens_get_uint_at(const gds_tokens_t * tks, unsigned int index,
			 unsigned int * value);

  // -----[ tokens_get_double_at ]-----------------------------------
  /**
   * Get a double token from the list.
   *
   * \param tks   is the list of tokens.
   * \param index is the position of the requested token.
   * \param value is a pointer to the token value.
   * \retval 0 in case the conversion was successful,
   *   <0 in case the token index is invalid or the conversion failed.
   */
  int tokens_get_double_at(const gds_tokens_t * tks, unsigned int index,
			   double * value);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_TOKENS_H__ */
