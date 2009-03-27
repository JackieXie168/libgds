// =================================================================
// @(#)tokenizer.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 10/07/2003
// $Id$
// =================================================================

/**
 * \file
 * Provide data structures and functions to perform basic character
 * line tokenization. The tokenizer features quoted blocks and
 * parameters replacement. This is a very specific lexer.
 *
 * Here is a typical example:
 * \code
 * unsigned int index;
 * const gds_tokens_t * tks;
 * gds_tokenizer_t * tk= tokenizer_create(" \t", "(", ")");
 * int error= tokenizer_run(tk, "Hello (poor world) !!!");
 * if (error != TOKENIZER_SUCCESS) {
 *   tokenizer_perror(gdserr, error);
 * } else {
 *   tks= tokenizer_get_tokens(tk);
 *   for (index= 0; index < tokens_get_num(tks); index++)
 *     stream_printf(gdsout, "%u \"%s\"\n", index,
 *                   tokens_get_string_at(tks, index));
 * }
 * tokenizer_destroy(&tk);
 * \endcode
 */

#ifndef __GDS_TOKENIZER_H__
#define __GDS_TOKENIZER_H__

#include <stdio.h>
#include <libgds/params.h>
#include <libgds/stream.h>
#include <libgds/str_util.h>
#include <libgds/tokens.h>

/**
 * Tokenization error codes.
 */
typedef enum {
  /** Tokenization was successful. */
  TOKENIZER_SUCCESS            =  0,
  /** An unexpected error occurred. */
  TOKENIZER_ERROR_UNEXPECTED   = -1,
  /** A quoted block is not closed. */
  TOKENIZER_ERROR_MISSING_CLOSE= -2,
  /** A quoted block close char was found without matching open. */
  TOKENIZER_ERROR_MISSING_OPEN = -3,
  /** An unknown escape character was encountered. */
  TOKENIZER_ERROR_ESCAPE       = -5,
  /** An unknown parameter was encountered. */
  TOKENIZER_ERROR_PARAM_UNDEF  = -6,
  /** An invalid parameter was encountered. */
  TOKENIZER_ERROR_PARAM_INVALID= -7,
} tokenizer_error_t;

// -----[ Options ]--------------------------------------------------
/** If set, each delimiter separates two fields. If two consecutive
    delimiters are found, then an empty field is created in-between. */
#define TOKENIZER_OPT_SINGLE_DELIM 0x01
/** If set, a final empty token is allowed after the last delimiter. */
#define TOKENIZER_OPT_EMPTY_FINAL  0x02
  
typedef struct {
  gds_tokens_t   * tokens;
  char           * delimiters;       // A string of field delimiters
  uint8_t          flags;            // Options
  char           * opening_quotes;   // String of opening quotes
  char           * closing_quotes;   // String of closing quotes
  char           * protect_quotes;   // String of quotes (protected block)
  param_lookup_t   lookup;
  str_buf_t      * tk_buf;
} gds_tokenizer_t;

#ifdef __cplusplus
extern "C" {
#endif

  // ----- tokenizer_create -----------------------------------------
  /**
   * Create a tokenizer.
   *
   * \param delimiters
   *   is the list of possible delimiters provided in a NUL-terminated
   *   string.
   *.\param opening_quotes
   *   is the list of opening quotes provided in a NUL-terminated string.
   * \param closing_quotes
   *   is the list of closing_quotes. This list must have the exact
   *   same length as \p opening_quotes. The ordering of the characters
   *   must be equal.
   */
  gds_tokenizer_t * tokenizer_create(const char * delimiters,
				     const char * opening_quotes,
				     const char * closing_quotes);

  // ----- tokenizer_destroy ----------------------------------------
  /**
   * Destroy a tokenizer.
   *
   * \param tk_ref is a pointer to the tokenizer to be destroyed.
   */
  void tokenizer_destroy(gds_tokenizer_t ** tk_ref);

  // ----- tokenizer_get_tokens -------------------------------------
  /**
   * Return the result of the tokenization.
   *
   * \param tk is the source tokenizer.
   * \retval the resulting tokens. The tokens are owned by the
   *   tokenizer and shouldn't be modified or freed.
   */
  const gds_tokens_t * tokenizer_get_tokens(gds_tokenizer_t * tk);

  // ----- tokenizer_run --------------------------------------------
  /**
   * Run the tokenizer.
   *
   * \param tk  is the tokenizer.
   * \param str is the character string to be parsed.
   */
  int tokenizer_run(gds_tokenizer_t * tk, const char * str);

  // -----[ tokenizer_step ]-----------------------------------------
  /* \todo
   * \code
   * int tokenizer_step(gds_tokenizer_t * tk, const char ** str_ref);
   * \endcode
   */

  // -----[ tokenizer_strerror ]-------------------------------------
  /**
   * Return a textual message for an error code.
   *
   * \param error is the error code.
   * \retval the corresponding error message.
   */
  const char * tokenizer_strerror(int error);

  // ----- tokenizer_perror -----------------------------------------
  void tokenizer_perror(gds_stream_t * stream, int error);

  // ----- tokenizer_get_num_token ----------------------------------
  /* DEPRECATED
     uint16_t tokenizer_get_num_tokens(gds_tokenizer_t * tk);
  */

  // -----[ tokenizer_set_flag ]------------------------------------
  void tokenizer_set_flag(gds_tokenizer_t * tk, uint8_t flag);
  // -----[ tokenizer_reset_flag ]----------------------------------
  void tokenizer_reset_flag(gds_tokenizer_t * tk, uint8_t flag);
  // -----[ tokenizer_set_lookup ]----------------------------------
  void tokenizer_set_lookup(gds_tokenizer_t * tk,
			    param_lookup_t lookup);
  // -----[ tokenizer_set_protect_quotes ]---------------------------
  void tokenizer_set_protect_quotes(gds_tokenizer_t * tk,
				    const char * quotes);
  
#ifdef __cplusplus
}
#endif

#endif /* __GDS_TOKENIZER_H__ */
