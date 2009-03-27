// ==================================================================
// @(#)params.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 17/09/2008
// $Id$
// ==================================================================

/**
 * \file
 * Provide data structures and functions to manage parameters
 * replacement in strings of text. These functions are typically used
 * in conjunction with an associative array or an hash table for
 * parameters lookup.
 *
 * Here is a small example with hard-coded lookup. First, the
 * parameters lookup function is defined:
 * \code
 * static const char * _params_lookup(const char * param, void * ctx)
 * {
 *   if (!strcmp("PARAM1", param)) {
 *     return "Hello";
 *   } else if (!strcmp("PARAM2", param)) {
 *     return "world";
 *   }
 *   return NULL;
 * }
 *
 * param_lookup_t lookup= {
 *   .lookup= my_lookup,
 *   .ctx= NULL
 * };
 * \endcode
 *
 * Then, replacements can be performed.
 * \code
 * char * replaced= NULL;
 * int result= params_replace("$PARAM1 $PARAM2 !!!", lookup, &replaced, 0);
 * if (result != PARAMS_SUCCESS) {
 *   params_perror();
 *   exit(EXIT_FAILURE);
 * }
 * fprintf(stdout, "%s\n", replaced);
 * fflush(stdout);
 * FREE(replaced);
 * \endcode
 */

#ifndef __GDS_PARAMS_H__
#define __GDS_PARAMS_H__

#include <ctype.h>

#include <libgds/stream.h>

typedef struct {
  const char * (*lookup)(const char * param, void * ctx);
  void       * ctx;
} param_lookup_t;

/** Possible error codes. */
typedef enum {
  /** Success. */
  PARAMS_SUCCESS         = 0,
  /** Undefined parameter (name not found). */
  PARAMS_ERROR_UNDEF     = -1,
  /** Invalid parameter name. */
  PARAMS_ERROR_INVALID   = -2,
} params_error_t;

#define PARAM_CHAR_START      '$'
#define PARAM_CHAR_EOS        '\0'
#define PARAM_CHAR_UNDERSCORE '_'

#define PARAM_OPT_ALLOW_UNDEF 0x01

// -----[ _is_param_first_char ]-------------------------------------
/**
 * \internal
 */
static inline int _is_param_first_char(char c)
{
  return (isalpha(c) || (c == PARAM_CHAR_UNDERSCORE));
}

// -----[ _is_param_char ]-------------------------------------------
/**
 * \internal
 */
static inline int _is_param_char(char c)
{
  return (_is_param_first_char(c) || isdigit(c));
}

#ifdef __cplusplus
extern "C" {
#endif
  
  // -----[ params_replace ]-----------------------------------------
  /**
   * Perform a parameters replacement.
   *
   * \param str      is the source text.
   * \param lookup   is the parameter lookup object.
   * \param replaced is the resulting string. This string is
   *   allocated on the heap and it is the responsibility of the
   *    caller to free it.
   * \param options  is a set of parameters replacement options.
   * \retval an error code.
   */
  int params_replace(const char * str, param_lookup_t lookup,
		     char ** replaced, int options);

  // -----[ params_strerror ]----------------------------------------
  /**
   * Convert an error code to a meaningful error message.
   *
   * \param error is the error code.
   * \retval an error message/
   */
  const char * params_strerror(params_error_t error);

  // -----[ params_perror ]--------------------------------------------
  /**
   * Print an error message.
   *
   * \param stream is the output stream.
   * \param error  is the error code.
   */
  void params_perror(gds_stream_t * stream, params_error_t error);

  // -----[ default_lookup ]-------------------------------------------
  const char * default_lookup(const char * param, void * ctx);


#ifdef __cplusplus
}
#endif

#endif /* __GDS_PARAMS_H__ */
