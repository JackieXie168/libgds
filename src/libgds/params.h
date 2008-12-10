// ==================================================================
// @(#)params.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 17/09/2008
// $Id$
// ==================================================================

#ifndef __GDS_PARAMS_H__
#define __GDS_PARAMS_H__

#include <ctype.h>

#include <libgds/stream.h>

typedef struct {
  const char * (*lookup)(const char * param, void * ctx);
  void       * ctx;
} param_lookup_t;

typedef enum {
  PARAMS_SUCCESS         = 0,
  PARAMS_ERROR_UNDEF     = -1,
  PARAMS_ERROR_INVALID   = -2,
} params_error_t;

#define PARAM_CHAR_START      '$'
#define PARAM_CHAR_EOS        '\0'
#define PARAM_CHAR_UNDERSCORE '_'

#define PARAM_OPT_ALLOW_UNDEF 0x01

// -----[ _is_param_first_char ]-------------------------------------
static inline int _is_param_first_char(char c)
{
  return (isalpha(c) || (c == PARAM_CHAR_UNDERSCORE));
}

// -----[ _is_param_char ]-------------------------------------------
static inline int _is_param_char(char c)
{
  return (_is_param_first_char(c) || isdigit(c));
}

#ifdef __cplusplus
extern "C" {
#endif
  
  // -----[ params_replace ]-----------------------------------------
  int params_replace(const char * str, param_lookup_t lookup,
		     char ** replaced, int options);
  // -----[ params_strerror ]----------------------------------------
  const char * params_strerror(params_error_t error);
  // -----[ params_perror ]--------------------------------------------
  void params_perror(gds_stream_t * stream, params_error_t error);
  // -----[ default_lookup ]-------------------------------------------
  const char * default_lookup(const char * param, void * ctx);


#ifdef __cplusplus
}
#endif

#endif /* __GDS_PARAMS_H__ */
