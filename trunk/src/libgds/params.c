// ==================================================================
// @(#)params.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 17/09/2008
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <string.h>

#include <libgds/assoc_array.h>
#include <libgds/memory.h>
#include <libgds/params.h>
#include <libgds/str_util.h>

//#define __DEBUG_PARAMS__

// -----[ params_strerror ]------------------------------------------
const char * params_strerror(params_error_t error)
{
  switch (error) {
  case PARAMS_SUCCESS:
    return "success";
  case PARAMS_ERROR_UNDEF:
    return "undefined parameter";
  case PARAMS_ERROR_INVALID:
    return "invalid parameter name";
  }
  return NULL;
}

// -----[ params_perror ]--------------------------------------------
void params_perror(gds_stream_t * stream, params_error_t error)
{
  const char * msg= params_strerror(error);
  if (msg == NULL)
    stream_printf(stream, "unknown error (%d)", error);
  else
    stream_printf(stream, msg);
}

typedef enum {
  _STATE_NORMAL,         // Out of a parameter
  _STATE_IN_PARAM_FIRST, // First character of parameter
  _STATE_IN_PARAM,       // After first character of parameter
  _STATE_FINAL,          // Final state (EOS/error)
  _STATE_MAX,
} _parser_state_t;

#ifdef __DEBUG_PARAMS__
static char * _STATE_NAMES[_STATE_MAX]= {
  "NORMAL",
  "IN_PARAM_FIRST",
  "IN_PARAM",
  "FINAL",
};
#endif /* __DEBUG_PARAMS__ */

// -----[ params_replace ]-------------------------------------------
/**
 * Replace parameters starting with '$' in the string, using the
 * provided lookup function.
 *
 * Returns
 *   0 on success
 *  <0 on error
 */
int params_replace(const char * str, param_lookup_t lookup,
		   char ** replaced, int options)
{
  // ! Warning !
  // -----------
  //   Code is incorrectly generated by Darwin GCC 4.0.1 if the
  //   state variable is of enum type, as declared below:
  //     _parser_state_t state= _STATE_NORMAL;
  int state= _STATE_NORMAL;
  params_error_t error= PARAMS_SUCCESS;
  char * param;
  const char * param_start= NULL;
  const char * value;
  int dont_eat;
  str_buf_t * buf= str_buf_create(256);

  while (state != _STATE_FINAL) {

#ifdef __DEBUG_PARAMS__
    stream_printf(gdsout, "FSM:process(%s, '%c')\n",
		  _STATE_NAMES[state], *str);
#endif /* __DEBUG_PARAMS__ */

    dont_eat= 0;

    switch (state) {

    case _STATE_NORMAL:
      if (*str == PARAM_CHAR_EOS) {
	state= _STATE_FINAL;
      } else if (*str == PARAM_CHAR_START) {
	state= _STATE_IN_PARAM_FIRST;
      } else {
	str_buf_write_char(buf, *str);
      }
      break;

    case _STATE_IN_PARAM_FIRST:
      if (*str == PARAM_CHAR_EOS) {
	state= _STATE_FINAL;
      } else if (!_is_param_first_char(*str)) {
	error= PARAMS_ERROR_INVALID;
	state= _STATE_FINAL;
      } else {
	param_start= str;
	state= _STATE_IN_PARAM;
      }
      break;
      
    case _STATE_IN_PARAM:
      if (!_is_param_char(*str)) {
	param= MALLOC((str-param_start+1)*sizeof(char));
	memcpy(param, param_start, str-param_start);
	param[str-param_start]= '\0';	
	value= lookup.lookup(param, lookup.ctx);
	FREE(param);
	if (value == NULL) {
	  if (!(options & PARAM_OPT_ALLOW_UNDEF)) {
	    error= PARAMS_ERROR_UNDEF;
	    state= _STATE_FINAL;
	    break;
	  }
	} else
	  str_buf_write_string(buf, value);
	dont_eat= 1;
	state= _STATE_NORMAL;
      }
      break;
      
    default:
      abort();
    }

    // Advance in stream
    if ((*str != PARAM_CHAR_EOS) && (!dont_eat))
      str++;

  }

  if (error == PARAMS_SUCCESS) {
    // Terminate buffer
    str_buf_write_char(buf, '\0');
    // Mark as empty
    str_buf_reset(buf);

    *replaced= buf->data;
    buf->data= NULL;
  }

  str_buf_destroy(&buf);
  return error;
}

// -----[ default_lookup ]-------------------------------------------
const char * default_lookup(const char * param, void * ctx)
{
  gds_assoc_array_t * array= (gds_assoc_array_t *) ctx;
  return (char *) assoc_array_get(array, param);
}
