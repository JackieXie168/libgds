// =================================================================
// @(#)tokenizer.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 10/07/2003
// $Id$
// =================================================================

#ifndef __GDS_TOKENIZER_H__
#define __GDS_TOKENIZER_H__

#include <stdio.h>
#include <libgds/params.h>
#include <libgds/stream.h>
#include <libgds/str_util.h>
#include <libgds/tokens.h>

typedef enum {
  TOKENIZER_SUCCESS            =  0,
  TOKENIZER_ERROR_UNEXPECTED   = -1,
  TOKENIZER_ERROR_MISSING_CLOSE= -2,
  TOKENIZER_ERROR_MISSING_OPEN = -3,
  TOKENIZER_ERROR_ESCAPE       = -5,
  TOKENIZER_ERROR_PARAM_UNDEF  = -6,
  TOKENIZER_ERROR_PARAM_INVALID= -7,
  TOKENIZER_ERROR_PARAM_LOOKUP = -8,
} tokenizer_error_t;

// -----[ Options ]--------------------------------------------------
#define TOKENIZER_OPT_SINGLE_DELIM 0x01
//   If the SINGLE_DELIM option is set, each delimiter
//   separates two fields. If two consecutive delimiters are
//   found, then an empty field is created in-between.
#define TOKENIZER_OPT_EMPTY_FINAL  0x02
//   If the EMPTY_FINAL option is set, an final empty token is
//   allowed after the last delimiter.

  
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
  gds_tokenizer_t * tokenizer_create(const char * delimiters,
				     const char * opening_quotes,
				     const char * closing_quotes);
  // ----- tokenizer_destroy ----------------------------------------
  void tokenizer_destroy(gds_tokenizer_t ** tokenizer_ref);
  // ----- tokenizer_get_tokens -------------------------------------
  const gds_tokens_t * tokenizer_get_tokens(gds_tokenizer_t * tokenizer);
  // ----- tokenizer_run --------------------------------------------
  int tokenizer_run(gds_tokenizer_t * tokenizer, const char * string);
  // -----[ tokenizer_strerror ]---------------------------------------
  const char * tokenizer_strerror(int error);
  // ----- tokenizer_perror -----------------------------------------
  void tokenizer_perror(gds_stream_t * stream, int error);
  // ----- tokenizer_get_num_token ----------------------------------
  uint16_t tokenizer_get_num_tokens(gds_tokenizer_t * tokenizer);

  // -----[ tokenizer_set_flag ]------------------------------------
  void tokenizer_set_flag(gds_tokenizer_t * tokenizer, uint8_t flag);
  // -----[ tokenizer_reset_flag ]----------------------------------
  void tokenizer_reset_flag(gds_tokenizer_t * tokenizer, uint8_t flag);
  // -----[ tokenizer_set_lookup ]----------------------------------
  void tokenizer_set_lookup(gds_tokenizer_t * tokenizer,
			    param_lookup_t lookup);
  // -----[ tokenizer_set_protect_quotes ]---------------------------
  void tokenizer_set_protect_quotes(gds_tokenizer_t * tokenizer,
				    const char * quotes);
  
#ifdef __cplusplus
}
#endif

#endif /* __GDS_TOKENIZER_H__ */
