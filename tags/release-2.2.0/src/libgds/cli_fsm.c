// ==================================================================
// @(#)cli_fsm.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 02/12/2008
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgds/cli.h>
#include <libgds/cli_commands.h>
#include <libgds/cli_params.h>
#include <libgds/memory.h>

//#define DEBUG
#include <libgds/debug.h>

// -----[ FSM states ]-----------------------------------------------
#define STATE_CMD_FIRST 0
#define STATE_CMD       1
#define STATE_SUBCMD    2
#define STATE_ARGS      3
#define STATE_ARG       4
#define STATE_VARARG    6
#define STATE_OPT       7
#define STATE_UNKNOWN   8
#define STATE_INIT      STATE_CMD_FIRST

// -----[ Special characters ]---------------------------------------
#define CLI_DELIMITERS " \t"
#define CLI_OPENING_QUOTES "\"'"
#define CLI_CLOSING_QUOTES "\"'"
#define CLI_PROTECT_QUOTES "'"

// -----[ _cli_find_subcmd ]-----------------------------------------
/**
 * Search for the command named 'name' in the list of sub-commands
 * of 'cmd'. If not match was found, and if enabled by 'en_omni',
 * perform a lookup in the list of omnipresent commands.
 */
static inline
cli_cmd_t * _cli_find_subcmd(cli_t * cli, cli_cmd_t * cmd,
			     const char * name, int en_omni)
{
  cli_cmd_t * match= NULL;

  __debug("_cli_find_subcmd(cmd:\"%s\", name:\"%s\")\n", cmd->name, name);

  // Match regular sub-command
  match= cli_cmd_find_subcmd(cmd, name);

  // Match omnipresent command (if enabled/possible)
  if ((match == NULL) && en_omni)
    match= cli_cmd_find_subcmd(cli->omni_cmd, name);

  return match;
}

// -----[ _fsm_state2str ]-------------------------------------------
static inline const char * _fsm_state2str(int match)
{
  switch (match) {
  case STATE_CMD_FIRST:
    return "MATCH-CMD-FIRST";
  case STATE_CMD:
    return "MATCH-CMD";
  case STATE_SUBCMD:
    return "MATCH-SUBCMD";
  case STATE_ARGS:
    return "MATCH-ARGS";
  case STATE_ARG:
    return "MATCH-ARG";
  case STATE_VARARG:
    return "MATCH-VARARG";
  case STATE_OPT:
    return "MATCH-OPT";
  default:
    return "??? UNKNOWN ???";
  }
}

// -----[ _cli_cmd_reset ]-------------------------------------------
/**
 * Initialize the arguments and option values of the command.
 */
static inline void _cli_cmd_reset(cli_cmd_t * cmd)
{
  if (cmd->args != NULL)
    cli_args_clear(cmd->args);
  if (cmd->opts != NULL)
    cli_args_clear(cmd->opts);
}

// -----[ _cli_fsm_copy_token ]--------------------------------------
static inline char * _cli_fsm_copy_token(cli_fsm_t * fsm, const char * token)
{
  str_destroy(&fsm->token_copy);
  fsm->token_copy= str_create(token);
  return fsm->token_copy;
}

// -----[ _cli_fsm_init_tokens ]-------------------------------------
static inline int _cli_fsm_init_tokens(cli_t * cli, cli_fsm_t * fsm,
				       const char * line)
{
  int result;

  result= tokenizer_run(fsm->tokenizer, line);
  if (result < 0) {
    cli_set_user_error(cli, tokenizer_strerror(result));
    return CLI_ERROR_SYNTAX;
  }

  fsm->tokens= tokenizer_get_tokens(fsm->tokenizer);
  fsm->token_index= 0;
  return CLI_SUCCESS;
}

// -----[ _cli_fsm_has_more_token ]----------------------------------
static inline int _cli_fsm_has_more_token(cli_fsm_t * fsm)
{
  return (fsm->token_index < tokens_get_num(fsm->tokens));
}

// -----[ _cli_fsm_next_token ]--------------------------------------
static inline const char * _cli_fsm_next_token(cli_fsm_t * fsm)
{
  if (!_cli_fsm_has_more_token(fsm))
    return NULL;
  fsm->token_index++;
  return tokens_get_string_at(fsm->tokens, fsm->token_index-1);
}

// -----[ _cli_elem_unknown ]----------------------------------------
static inline void _cli_elem_unknown(cli_elem_t * elem) {
  elem->type= CLI_ELEM_UNKNOWN;
}

// -----[ _cli_elem_cmd ]--------------------------------------------
static inline void _cli_elem_cmd(cli_elem_t * elem, cli_cmd_t * cmd) {
  elem->type= CLI_ELEM_CMD;
  elem->cmd= cmd;
}

// -----[ _cli_elem_arg ]--------------------------------------------
static inline void _cli_elem_arg(cli_elem_t * elem, cli_arg_t * arg) {
  elem->type= CLI_ELEM_ARG;
  elem->arg= arg;
}

// -----[ _cli_elem_type2str ]---------------------------------------
static inline const char * _cli_elem_type2str(cli_elem_type_t type) {
  switch (type) {
  case CLI_ELEM_CMD:
    return "cmd";
  case CLI_ELEM_ARG:
    return "arg";
  case CLI_ELEM_UNKNOWN:
    return "unknown";
  default:
    abort();
  }
}

// -----[ _cli_fsm_next_state ]--------------------------------------
/**
 * CLI Finite State Machine (FSM). Possible transitions are as
 * follows:
 *
 *   CMD_FIRST ----> SUBCMD
 *   CMD       ----> SUBCMD
 *   SUBCMD    ----> ARGS
 *   ARGS      ----> ARG
 *   ARGS      ----> OPT
 *   ARGS      ----> VARARG
 *   ARGS      ----> CMD
 *
 * Initial state (INIT) is CMD_FIRST
 */
static inline int _cli_fsm_next_state(cli_t * cli, cli_fsm_t * fsm,
				      const char * token, int compl)
{
  cli_cmd_t * sub_cmd= NULL;
  cli_arg_t * arg, * opt;
  int error;
  int next_state= STATE_UNKNOWN;
  char * name= NULL;
  char * value= NULL;

  fsm->consume= 0;

  __debug("_cli_fsm_next_state(token:[%s],state:[%s])\n",
	  token, _fsm_state2str(fsm->state));
  
  switch (fsm->state) {

  case STATE_CMD_FIRST:
    _cli_elem_cmd(&fsm->elem, fsm->cmd);
    next_state= STATE_SUBCMD;
    break;
    
  case STATE_CMD:
    _cli_elem_cmd(&fsm->elem, fsm->cmd);
    if (fsm->cmd->type == CLI_CMD_TYPE_STD)
      return CLI_ERROR_UNKNOWN_CMD;
    error= cli_cmd_enter(cli, fsm->cmd, 0);
    if (error)
      return error;
    next_state= STATE_SUBCMD;
    break;

  case STATE_SUBCMD:
    if (compl)
      return CLI_ERROR_UNKNOWN_CMD;
    sub_cmd= _cli_find_subcmd(cli, fsm->cmd, token,
			      fsm->token_index == 1);
    if (sub_cmd == NULL)
      return CLI_ERROR_UNKNOWN_CMD;
    fsm->consume= 1;
    fsm->cmd= sub_cmd;
    if (sub_cmd->type == CLI_CMD_TYPE_PFX) {
      next_state= STATE_CMD;
    } else {
      _cli_elem_cmd(&fsm->elem, fsm->cmd);
      _cli_cmd_reset(sub_cmd);
      fsm->param_index= 0;
      cli_args_bounds(sub_cmd->args, &fsm->param_min, &fsm->param_max);
      next_state= STATE_ARGS;
    }
    break;

  case STATE_ARGS:
    if (!strncmp(token, "--", 2)) {
      next_state= STATE_OPT;
    } else if (fsm->param_index < fsm->param_max) {
      if (fsm->param_index < fsm->param_min)
	next_state= STATE_ARG;
      else
	next_state= STATE_VARARG;
    } else {
      next_state= STATE_CMD;
    }
    break;

  case STATE_ARG:
    arg= cli_args_at(fsm->cmd->args, fsm->param_index);
    if (compl) {
      _cli_elem_arg(&fsm->elem, arg);
      return CLI_ERROR_MISSING_ARG;
    }
    error= cli_arg_set_value(arg, token);
    if (error)
      return error;
    fsm->param_index++;
    next_state= STATE_ARGS;
    fsm->consume= 1;
    break;

  case STATE_VARARG:
    arg= cli_args_at(fsm->cmd->args,
		     cli_args_num(fsm->cmd->args)-1);
    if ((error= cli_arg_set_value(arg, token)) < 0)
      return error;
    fsm->param_index++;
    next_state= STATE_ARGS;
    fsm->consume= 1;
    break;
    
  case STATE_OPT:
    if (fsm->cmd->opts == NULL)
      return CLI_ERROR_UNKNOWN_OPT;
    value= _cli_fsm_copy_token(fsm, token+2);
    name= strsep(&value, "=");
    opt= cli_opts_find(fsm->cmd->opts, name);
    if (opt == NULL)
      return CLI_ERROR_UNKNOWN_OPT;
    if (value == NULL) {
      if (compl)
	return CLI_ERROR_UNKNOWN_OPT;
      if (opt->need_value)
	return CLI_ERROR_MISSING_OPT_VALUE;
    } else {
      if (!opt->need_value)
	return CLI_ERROR_OPT_NO_VALUE;
      if (compl || (*value == '\0')) {
	_cli_elem_arg(&fsm->elem, opt);
	return CLI_ERROR_MISSING_OPT_VALUE;
      }
    }
    error= cli_arg_set_value(opt, value);
    if (error)
      return error;
    fsm->consume= 1;
    next_state= STATE_ARGS;
    break;

  default:
    gds_fatal("Invalid FSM state %d\n", fsm->state);
  }

  fsm->state= next_state;
  
  __debug(" +--> next_state:%s\n", _fsm_state2str(fsm->state));

  return CLI_SUCCESS;
}

// -----[ cli_fsm_create ]-------------------------------------------
cli_fsm_t * cli_fsm_create()
{
  cli_fsm_t * fsm= MALLOC(sizeof(cli_fsm_t));
  fsm->tokenizer= tokenizer_create(CLI_DELIMITERS,
				   CLI_OPENING_QUOTES,
				   CLI_CLOSING_QUOTES);
  tokenizer_set_protect_quotes(fsm->tokenizer, CLI_PROTECT_QUOTES);
  fsm->token_copy= NULL;
  return fsm;
}

// -----[ cli_fsm_destroy ]------------------------------------------
void cli_fsm_destroy(cli_fsm_t ** fsm_ref)
{
  cli_fsm_t * fsm= *fsm_ref;

  if (fsm != NULL) {
    tokenizer_destroy(&fsm->tokenizer);
    str_destroy(&fsm->token_copy);
    FREE(fsm);
    *fsm_ref= NULL;
  }
}

// -----[ _cli_fsm_reset ]-------------------------------------------
static inline void _cli_fsm_reset(cli_fsm_t * fsm, cli_cmd_t * cmd)
{
  fsm->state= STATE_INIT;
  fsm->cmd= cmd;
  fsm->consume= 1;
}

// -----[ cli_fsm_run ]----------------------------------------------
/**
 * Match the given string with the CLI command tree.
 */
int cli_fsm_run(cli_t * cli, cli_cmd_t * cmd, const char * line,
		const char * compl, cli_elem_t * elem)
{
  const char * token= NULL;
  int error;
  cli_fsm_t * fsm= cli->fsm;

  if (cmd == NULL)
    cli_get_cmd_context(cli, &cmd, NULL);

  _cli_fsm_reset(cli->fsm, cmd);

  // Initialize current element
  _cli_elem_cmd(&fsm->elem, cmd);

  // Tokenize input
  error= _cli_fsm_init_tokens(cli, fsm, line);
  if (error != 0)
    return error;

  __debug("cli_fsm_run\n"
	  " +-- context   :\"%s\"\n"
	  " +-- input     :\"%s\"\n"
	  " +-- num_tokens:%d\n",
	  cmd->name, line, tokens_get_num(fsm->tokens));
  if (compl != NULL)
    __debug(" +-- compl     :\"%s\"\n", compl);
  
  // Try to match all tokens (FSM)...
  while (error == CLI_SUCCESS) {
    if (fsm->consume) {
      if (!_cli_fsm_has_more_token(fsm))
	break;
      token= _cli_fsm_next_token(fsm);
    }
    error= _cli_fsm_next_state(cli, fsm, token, 0);
  }

  // A completion attempt can be made in case the prefix
  // was parsed by the FSM without error.
  if (compl != NULL) {
    __debug(" +-- COMPLETION ATTEMPT: [%s]\n", compl);
    if (error != CLI_SUCCESS)
      return CLI_ERROR_COMPL_FAILED;
    do {
      fsm->consume= 0;
      error=_cli_fsm_next_state(cli, fsm, (char*) compl, 1);
      if (error != CLI_SUCCESS)
	break;
    } while (!fsm->consume);
    __debug(" +-- result:%s\n"
	    "   +-- type:%s\n",
	    cli_strerror(error), _cli_elem_type2str(fsm->elem.type));
    switch (fsm->elem.type) {
    case CLI_ELEM_CMD:
      __debug("   +-- cmd:\"%s\"\n", fsm->elem.cmd->name);
      break;
    case CLI_ELEM_ARG:
      __debug("   +-- arg:\"%s\"\n", fsm->elem.arg->name);
      break;
    default:
      __debug("   +-- unknown\n");
    }
    *elem= fsm->elem;
    return error;
  }

  // Final check for errors:
  // - not enough parameters
  if ((error == CLI_SUCCESS) &&
      (fsm->state == STATE_ARGS) &&
      (fsm->param_index < fsm->param_min)) {
    error= CLI_ERROR_MISSING_ARG;
    _cli_elem_arg(&fsm->elem, cli_args_at(fsm->cmd->args, fsm->param_index));
  }

  __debug(" +-- result:%s\n   +-- cmd:%s\n",
	  cli_strerror(error), fsm->cmd->name);
  _cli_elem_cmd(elem, fsm->cmd);
  return error;
 }
