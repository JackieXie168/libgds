// =================================================================
// @(#)tokenizer.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 10/07/2003
// $Id$
// =================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <string.h>

#include <libgds/memory.h>
#include <libgds/stack.h>
#include <libgds/str_util.h>
#include <libgds/tokenizer.h>

//#define __DEBUG_TOKENIZER__

// Internal constants
#define TK_BUF_SIZE 1024       // Initial size of token buffer
#define STACK_MAX_DEPTH 100    // Maximum depth for state stack

#define TOKENIZER_CHAR_ESCAPE '\\'
#define TOKENIZER_CHAR_EOL    '\n'
#define TOKENIZER_CHAR_EOS    '\0'

typedef enum {
  _STATE_NORMAL,     // Initial parsing state (out of block, escape)
  _STATE_IN_DELIM,   // Currently parsing delimiters
  _STATE_IN_BLOCK,   // Currently in a block
  _STATE_FROM_BLOCK, // After block state (used to merge adjacent blocks)
  _STATE_IN_ESCAPE,  // Currently parsing an escaped character
  _STATE_IN_PARAM_FIRST,
  _STATE_IN_PARAM,
  _STATE_FINAL,      // Final state (success/error)
  _STATE_MAX,
} _state_t;

#ifdef __DEBUG_TOKENIZER__
static char * _STATE_NAMES[_STATE_MAX]= {
  "NORMAL",
  "IN_DELIM",
  "IN_BLOCK",
  "FROM_BLOCK",
  "IN_ESCAPE",
  "IN_PARAM_FIRST",
  "IN_PARAM",
  "FINAL",
};
#endif /* __DEBUG_TOKENIZER__ */

// -----[ _assert_disjoint_charsets ]--------------------------------
static inline void _assert_disjoint_charsets(const char * set1,
					     const char * set2)
{
  unsigned int index, index2;
  for (index= 0; index < strlen(set1); index++)
    for (index2= 0; index2 < strlen(set2); index2++)
    assert(set1[index] != set2[index2]);
}

// ----- tokenizer_create -------------------------------------------
/**
 *
 */
gds_tokenizer_t * tokenizer_create(const char * delimiters,
				   const char * opening_quotes,
				   const char * closing_quotes)
{
  gds_tokenizer_t * tokenizer=
    (gds_tokenizer_t *) MALLOC(sizeof(gds_tokenizer_t));
  tokenizer->tokens= NULL;
  tokenizer->delimiters= str_create(delimiters);
  tokenizer->flags= 0;
  tokenizer->protect_quotes= NULL;

  // Initialize quotes
  if ((opening_quotes != NULL) && (closing_quotes != NULL)) {

    // Check that the number of opening quotes equals the
    // number of closing quotes.
    assert(strlen(opening_quotes) == strlen(closing_quotes));
    // Check that there is no overlap between delimiters,
    // opening/closing quotes.
    _assert_disjoint_charsets(delimiters, opening_quotes);
    _assert_disjoint_charsets(delimiters, closing_quotes);

    tokenizer->opening_quotes= str_create(opening_quotes);
    tokenizer->closing_quotes= str_create(closing_quotes);
  } else {
    tokenizer->opening_quotes= str_create("");
    tokenizer->closing_quotes= str_create("");
  }

  // Token buffer used for parsing (initially empty)
  tokenizer->tk_buf= str_buf_create(TK_BUF_SIZE);

  // Parameter lookup function
  tokenizer->lookup.lookup= NULL;
  tokenizer->lookup.ctx= NULL;

  return tokenizer;
}

// ----- tokenizer_destroy ------------------------------------------
/**
 *
 */
void tokenizer_destroy(gds_tokenizer_t ** tokenizer_ref)
{
  gds_tokenizer_t * tokenizer= *tokenizer_ref;
  if (tokenizer != NULL) {
    str_destroy(&tokenizer->delimiters);
    str_destroy(&tokenizer->opening_quotes);
    str_destroy(&tokenizer->closing_quotes);
    str_destroy(&tokenizer->protect_quotes);
    tokens_destroy(&tokenizer->tokens);
    str_buf_destroy(&tokenizer->tk_buf);
    FREE(tokenizer);
    *tokenizer_ref= NULL;
  }
}

// ----- tokenizer_get_tokens ---------------------------------------
/**
 *
 */
const gds_tokens_t * tokenizer_get_tokens(gds_tokenizer_t * tokenizer)
{
  return tokenizer->tokens;
}

// -----[ _is_delimiter ]--------------------------------------------
static inline int _is_delimiter(gds_tokenizer_t * tokenizer, char c)
{
  return (!(c == TOKENIZER_CHAR_EOS) &&
	  (strchr(tokenizer->delimiters, c) != NULL));
}

// -----[ _is_opening_quote ]----------------------------------------
static inline int _is_opening_quote(gds_tokenizer_t * tokenizer, char c,
				    char * closing_quote, int * protecting)
{
  char * match;
  if (c == TOKENIZER_CHAR_EOS)
    return 0;
  if (tokenizer->opening_quotes == NULL)
    return 0;
  match= strchr(tokenizer->opening_quotes, c);
  if (match == NULL)
    return 0;

  // Return matching closing quote (if requested)
  if (closing_quote != NULL)
    *closing_quote= *(tokenizer->closing_quotes+
		      (match-tokenizer->opening_quotes));

  // Check if this is a "protecting" quote (if requested)
  if (protecting != NULL) {
    if ((tokenizer->protect_quotes != NULL) &&
	(strchr(tokenizer->protect_quotes, c) != NULL))
      *protecting= 1;
    else
      *protecting= 0;
  }

  return 1;
}

// -----[ _is_closing_quote ]----------------------------------------
static inline int _is_closing_quote(gds_tokenizer_t * tokenizer, char c,
				    char * expected_quote)
{
  char * match;
  if (c == TOKENIZER_CHAR_EOS)
    return 0;
  if (tokenizer->closing_quotes == NULL)
    return 0;
  match= strchr(tokenizer->closing_quotes, c);
  if (match == NULL)
    return 0;
  if ((expected_quote != NULL) && (*match != *expected_quote))
    return 0;
  return 1;
}

// -----[ _peek_buf ]------------------------------------------------
static inline char * _peek_buf(gds_tokenizer_t * tokenizer)
{
  // Terminate buffer
  str_buf_write_char(tokenizer->tk_buf, '\0');
  // Mark as empty
  str_buf_reset(tokenizer->tk_buf);

#ifdef __DEBUG_TOKENIZER__
  stream_printf(gdsout, "FSM:peek_buf \"%s\"\n", tokenizer->tk_buf->data);
#endif /* __DEBUG_TOKENIZER__ */

  return tokenizer->tk_buf->data;
}

// -----[ _push_state ]----------------------------------------------
static inline void _push_state(gds_stack_t * stack, _state_t state)
{
#ifdef __DEBUG_TOKENIZER__
  stream_printf(gdsout, "FSM:push_state(%s)\n", _STATE_NAMES[state]);
#endif /* __DEBUG_TOKENIZER__ */
  stack_push(stack, (void *) state);
}

// -----[ _pop_state ]-----------------------------------------------
static inline _state_t _pop_state(gds_stack_t * stack)
{
  _state_t state;
  assert(!stack_is_empty(stack));
  state= (_state_t) stack_pop(stack);
#ifdef __DEBUG_TOKENIZER__
  stream_printf(gdsout, "FSM:pop_state(): %s\n", _STATE_NAMES[state]);
#endif /* __DEBUG_TOKENIZER__ */
  return state;
}

// -----[ _escape ]--------------------------------------------------
static inline char _escape(char c)
{
  switch (c) {
  case 'a': return '\a';
  case 'e': return '\033';
  case 'r': return '\r';
  case 't': return '\t';
  case 'n': return '\n';
  default:
    return c;
  }
}

// ----- tokenizer_run ----------------------------------------------
/**
 *
 */
int tokenizer_run(gds_tokenizer_t * tokenizer, const char * str)
{
  int error= TOKENIZER_SUCCESS;
  _state_t state= _STATE_IN_DELIM;
  gds_stack_t * stack= stack_create(STACK_MAX_DEPTH);
  char closing_quote= 0;
  int dont_eat;
  int protecting= 0;
  char * param;
  const char * param_start= NULL;
  const char * value;

  // Allocate list of tokens (free previous one if needed)
  if (tokenizer->tokens != NULL)
    tokens_destroy(&tokenizer->tokens);
  tokenizer->tokens= tokens_create();

  // Reset buffer
  str_buf_reset(tokenizer->tk_buf);

  // Finite State Machine
  while (state != _STATE_FINAL) {

#ifdef __DEBUG_TOKENIZER__
    stream_printf(gdsout, "FSM:process(%s, '%c')\n",
		  _STATE_NAMES[state], *str);
#endif /* __DEBUG_TOKENIZER__ */

    dont_eat= 0;

    // Finite State Machine
    switch (state) {

    case _STATE_NORMAL:
      if (*str == TOKENIZER_CHAR_ESCAPE) {
	_push_state(stack, state);
	state= _STATE_IN_ESCAPE;
      } else if ((tokenizer->lookup.lookup != NULL) &&
		 (*str == PARAM_CHAR_START)) {
	_push_state(stack, state);
	state= _STATE_IN_PARAM_FIRST;
      } else if ((*str == TOKENIZER_CHAR_EOS) ||
		 (*str == TOKENIZER_CHAR_EOL)) {
	tokens_add_copy(tokenizer->tokens, _peek_buf(tokenizer));
	state= _STATE_FINAL;
      } else if (_is_delimiter(tokenizer, *str)) {
	tokens_add_copy(tokenizer->tokens, _peek_buf(tokenizer));
	state= _STATE_IN_DELIM;
      } else if (_is_opening_quote(tokenizer, *str, &closing_quote,
				   &protecting)) {
	state= _STATE_IN_BLOCK;
      } else if (_is_closing_quote(tokenizer, *str, NULL)) {
	error= TOKENIZER_ERROR_MISSING_OPEN;
	state= _STATE_FINAL;
      } else
	str_buf_write_char(tokenizer->tk_buf, *str);
      break;

    case _STATE_IN_DELIM:
      if (*str == TOKENIZER_CHAR_EOS) {
	if (tokenizer->flags & TOKENIZER_OPT_EMPTY_FINAL) {
	  dont_eat= 1;
	  state= _STATE_NORMAL;
	} else
	  state= _STATE_FINAL;
      } else if (_is_delimiter(tokenizer, *str)) {
	// Stay in DELIM state:
	//   If the SINGLE_DELIM option is set, each delimiter
	//   separates two fields. If two consecutive delimiters are
	//   found, then an empty field must be created.
	if (tokenizer->flags & TOKENIZER_OPT_SINGLE_DELIM) {
	  tokens_add_copy(tokenizer->tokens, "");
	}
      } else {
	dont_eat= 1;
	state= _STATE_NORMAL;
      }
      break;

    case _STATE_IN_BLOCK:
      if (*str == TOKENIZER_CHAR_ESCAPE) {
	_push_state(stack, state);
	state= _STATE_IN_ESCAPE;
      } else if ((tokenizer->lookup.lookup != NULL) &&
		 (*str == PARAM_CHAR_START) &&
		 !protecting) {
	_push_state(stack, state);
	state= _STATE_IN_PARAM_FIRST;
      } else if (*str == TOKENIZER_CHAR_EOS) {
	error= TOKENIZER_ERROR_MISSING_CLOSE;
	state= _STATE_FINAL;
      } else if (_is_closing_quote(tokenizer, *str, &closing_quote)) {
	state= _STATE_FROM_BLOCK;
      } else
	str_buf_write_char(tokenizer->tk_buf, *str);
      break;

    case _STATE_FROM_BLOCK:
      if (_is_opening_quote(tokenizer, *str, &closing_quote, &protecting)) {
	state= _STATE_IN_BLOCK;
      } else {
	dont_eat= 1;
	state= _STATE_NORMAL;
      }
      break;

    case _STATE_IN_ESCAPE:
      state= _pop_state(stack);
      if (*str == TOKENIZER_CHAR_EOS) {
	error= TOKENIZER_ERROR_ESCAPE;
	state= _STATE_FINAL;
	break;
      }
      str_buf_write_char(tokenizer->tk_buf, _escape(*str));
      break;

    case _STATE_IN_PARAM_FIRST:
      if (*str == TOKENIZER_CHAR_EOS) {
	state= _STATE_FINAL;
      } else if (!_is_param_first_char(*str)) {
	error= TOKENIZER_ERROR_PARAM_INVALID;
	state= _STATE_FINAL;
      } else {
	param_start= str; // Keep pointer to start of param
	state= _STATE_IN_PARAM;
      }
      break;

    case _STATE_IN_PARAM:
      if (!_is_param_char(*str)) {
	/*if (tokenizer->lookup.lookup == NULL) {
	  error= TOKENIZER_ERROR_PARAM_LOOKUP;
	  state= _STATE_FINAL;
	  break;
	  }*/
	param= MALLOC((str-param_start+1)*sizeof(char));
	memcpy(param, param_start, str-param_start);
	param[str-param_start]= '\0';
#ifdef __DEBUG_TOKENIZER__
	stream_printf(gdsout, "lookup(%s)\n", param);
#endif /* __DEBUG_TOKENIZER__ */
	if (tokenizer->lookup.lookup != NULL) {
	  value= tokenizer->lookup.lookup(param, tokenizer->lookup.ctx);
#ifdef __DEBUG_TOKENIZER__
	  stream_printf(gdsout, "-> value=%s\n", value);
#endif /* __DEBUG_TOKENIZER__ */
	  if (value == NULL) {
	    error= TOKENIZER_ERROR_PARAM_UNDEF;
	    state= _STATE_FINAL;
	    FREE(param);
	    break;
	  }
	  str_buf_write_string(tokenizer->tk_buf, value);
	} else {
	  str_buf_write_char(tokenizer->tk_buf, '$');
	  str_buf_write_string(tokenizer->tk_buf, param);
	}
	FREE(param);
	dont_eat= 1;
	state= _pop_state(stack);
      }
      break;
      
    default:
      abort();
    }

    if ((*str != TOKENIZER_CHAR_EOS) && (!dont_eat))
      str++;

  }

  // In case of success, state stack should be empty
  if (error == TOKENIZER_SUCCESS) {
    assert(stack_is_empty(stack));
  } else {
#ifdef __DEBUG_TOKENIZER__
    stream_printf(gdsout, "FSM:finish_with_error(%s)\n",
		  tokenizer_strerror(error));
#endif /* __DEBUG_TOKENIZER__ */
  }

  stack_destroy(&stack);
  return error;
}

// -----[ tokenizer_strerror ]---------------------------------------
const char * tokenizer_strerror(int error)
{
  switch (error) {
  case TOKENIZER_SUCCESS:
    return "success";
  case TOKENIZER_ERROR_UNEXPECTED:
    return "unexpected error\n";
  case TOKENIZER_ERROR_MISSING_CLOSE:
    return "missing close-quote";
  case TOKENIZER_ERROR_MISSING_OPEN:
    return "missing open-quote";
  case TOKENIZER_ERROR_ESCAPE:
    return "incomplete escape-sequence";
  case TOKENIZER_ERROR_PARAM_UNDEF:
    return "undefined parameter";
  case TOKENIZER_ERROR_PARAM_INVALID:
    return "invalid parameter name";
  case TOKENIZER_ERROR_PARAM_LOOKUP:
    return "no parameter lookup function configured";
  }
  return NULL;
}

// ----- tokenizer_perror -------------------------------------------
/**
 *
 */
void tokenizer_perror(gds_stream_t * stream, int error)
{
  const char * msg= tokenizer_strerror(error);
  if (msg != NULL)
    stream_printf(stream, msg);
  else
    stream_printf(stream, "unknown error (%d)", error);
}

// ----- tokenizer_get_num_token ------------------------------------
/**
 *
 */
uint16_t tokenizer_get_num_tokens(gds_tokenizer_t * tokenizer)
{
  if (tokenizer->tokens != NULL)
    return tokens_get_num(tokenizer->tokens);
  else
    return 0;
}

// -----[ tokenizer_set_flag ]---------------------------------------
void tokenizer_set_flag(gds_tokenizer_t * tokenizer, uint8_t flag)
{
  tokenizer->flags|= flag;
}

// -----[ tokenizer_reset_flag ]-------------------------------------
void tokenizer_reset_flag(gds_tokenizer_t * tokenizer, uint8_t flag)
{
  tokenizer->flags&= ~flag;
}

// -----[ tokenizer_set_lookup ]----------------------------------
void tokenizer_set_lookup(gds_tokenizer_t * tokenizer,
			  param_lookup_t lookup)
{
  tokenizer->lookup= lookup;
}

// -----[ tokenizer_set_protect_quotes ]-----------------------------
void tokenizer_set_protect_quotes(gds_tokenizer_t * tokenizer,
				  const char * quotes)
{
  str_destroy(&tokenizer->protect_quotes);
  if (quotes != NULL)
    tokenizer->protect_quotes= str_create(quotes);
}
