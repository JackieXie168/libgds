// ==================================================================
// @(#)cli.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <libgds/cli.h>
#include <libgds/cli_ctx.h>
#include <libgds/stream.h>
#include <libgds/memory.h>
#include <libgds/str_util.h>

#define CLI_DELIMITERS " \t"
#define CLI_OPENING_QUOTES "\"'"
#define CLI_CLOSING_QUOTES "\"'"
#define CLI_PROTECT_QUOTES "'"

static inline void _cli_cmds_set_parent(cli_cmds_t * cmds,
					cli_cmd_t * parent);

/////////////////////////////////////////////////////////////////////
//
// COMMANDS MANAGMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// ----- _cli_cmd_init ----------------------------------------------
/**
 * Initialize the parameters and option values of the command.
 */
static void _cli_cmd_init(cli_cmd_t * cmd)
{
  tokens_destroy(&cmd->param_values);
  if (cmd->options != NULL)
    cli_options_init(cmd->options);
}

// -----[ _cli_cmd_create ]------------------------------------------
static inline
cli_cmd_t * _cli_cmd_create(char * name,
			    FCliCommand fCommand,
			    FCliContextCreate fCtxCreate,
			    FCliContextDestroy fCtxDestroy,
			    cli_cmds_t * sub_cmds,
			    cli_params_t * params)
{
  cli_cmd_t * cmd= (cli_cmd_t *) MALLOC(sizeof(cli_cmd_t));

  cmd->name= (char *) MALLOC(sizeof(char)*(strlen(name)+1));
  strcpy(cmd->name, name);
  cmd->options= NULL;
  cmd->params= params;
  cmd->sub_cmds= sub_cmds;

  cmd->parent= NULL;
  cmd->fCtxCreate= fCtxCreate;
  cmd->fCtxDestroy= fCtxDestroy;
  cmd->fCommand= fCommand;
  cmd->help= NULL;
  cmd->param_values= NULL;

  if (sub_cmds != NULL)
    _cli_cmds_set_parent(sub_cmds, cmd);
  return cmd;
}

// ----- cli_cmd_create ---------------------------------------------
/**
 * Create a CLI command (executable).
 */
cli_cmd_t * cli_cmd_create(char * name,
			   FCliCommand fCommand,
			   cli_cmds_t * sub_cmds,
			   cli_params_t * params)
{
  return _cli_cmd_create(name,
			 fCommand,
			 NULL, NULL,
			 sub_cmds, params);
}

// ----- cli_cmd_create_ctx -----------------------------------------
/**
 * Create a CLI context command (not executable).
 */
cli_cmd_t * cli_cmd_create_ctx(char * name,
			       FCliContextCreate fCtxCreate,
			       FCliContextDestroy fCtxDestroy,
			       cli_cmds_t * sub_cmds,
			       cli_params_t * params)
{
  return _cli_cmd_create(name, NULL,
			 fCtxCreate, fCtxDestroy,
			 sub_cmds, params);
}

// ----- cli_cmd_destroy --------------------------------------------
/**
 * Destroy a CLI command.
 */
void cli_cmd_destroy(cli_cmd_t ** cmd_ref)
{
  if (*cmd_ref != NULL) {
    FREE((*cmd_ref)->name);
    ptr_array_destroy(&(*cmd_ref)->sub_cmds);
    cli_params_destroy(&(*cmd_ref)->params);
    cli_options_destroy(&(*cmd_ref)->options);
    tokens_destroy(&(*cmd_ref)->param_values);
    FREE(*cmd_ref);
    *cmd_ref= NULL;
  }
}

// ----- cli_cmd_dump -----------------------------------------------
/**
 *
 */
void cli_cmd_dump(gds_stream_t * stream, char * prefix, cli_cmd_t * cmd)
{
  unsigned int index;
  cli_cmd_t * sub_cmd;
  cli_option_t * option;
  cli_param_t * param;
  char * pcNewPrefix;

  stream_printf(stream, "%s%s", prefix, cmd->name);
  if (cmd->options != NULL) {
    for (index= 0; index < ptr_array_length(cmd->options); index++) {
      option= (cli_option_t *) cmd->options->data[index];
      stream_printf(stream, " [--%s]", option->name);
    }
  }
  if (cmd->params != NULL)
    for (index= 0; index < ptr_array_length(cmd->params); index++) {
      param= (cli_param_t *) cmd->params->data[index];
      stream_printf(stream, " %s", param->name);
      if (param->type == CLI_PARAM_TYPE_VARARG) {
	if (param->max_args > 0)
	  stream_printf(stream, "?(0-%d)", param->max_args);
	else
	  stream_printf(stream, "?(0-any)");
      }
    }
  stream_printf(stream, "\n");
  if (cmd->sub_cmds != NULL) {
    pcNewPrefix= (char *) MALLOC(sizeof(char)*(strlen(prefix)+3));
    strcpy(pcNewPrefix, prefix);
    strcat(pcNewPrefix, "  ");
    for (index= 0; index < ptr_array_length(cmd->sub_cmds); index++) {
      sub_cmd= (cli_cmd_t *) cmd->sub_cmds->data[index];
      cli_cmd_dump(stream, pcNewPrefix, sub_cmd);
    }
    FREE(pcNewPrefix);
  }
}

// ----- _cli_cmds_item_compare -------------------------------------
/**
 * Compare two commands based on their names. The alphanumeric
 * ordering is used for the comparison. This is used for sorting
 * commands in the CLI command tree.
 */
static int _cli_cmds_item_compare(const void * item1,
				  const void * item2,
				  unsigned int elt_size)
{
  cli_cmd_t * cmd1= *((cli_cmd_t **) item1);
  cli_cmd_t * cmd2= *((cli_cmd_t **) item2);

  return strcmp(cmd1->name, cmd2->name);
}

// ----- _cli_cmds_item_destroy -------------------------------------
/**
 * Free a command in a list of commands.
 */
static void _cli_cmds_item_destroy(void * item, const void * ctx)
{
  cli_cmd_destroy((cli_cmd_t **) item);
}

// ----- cli_cmds_create --------------------------------------------
/**
 * Create a list of command. The commands in the list are sorted based
 * on the alphanumeric ordering (thanks to the _cli_cmds_item_compare
 * function).
 */
cli_cmds_t * cli_cmds_create()
{
  return (cli_cmds_t *) ptr_array_create(ARRAY_OPTION_SORTED,
					 _cli_cmds_item_compare,
					 _cli_cmds_item_destroy,
					 NULL);
}

// ----- cli_cmds_destroy -------------------------------------------
/**
 * Destroy a list of commands.
 *
 * NOTE: all the commands in the list are also freed thanks to the
 * underlying array's destroy command. It is also safe to use this
 * function on lists returned by the cli_matching_cmds function since
 * in this case, the underlying array's destroy function is NULL.
 */
void cli_cmds_destroy(cli_cmds_t ** cmds_ref)
{
  ptr_array_destroy((SPtrArray **) cmds_ref);
}

// ----- cli_cmds_add -----------------------------------------------
/**
 * Add a command (pCmd) in the list (pCmds).
 */
int cli_cmds_add(cli_cmds_t * cmds, cli_cmd_t * cmd)
{
  return ptr_array_add((SPtrArray *) cmds, &cmd);
}

// -----[ cli_cmds_get_num ]-----------------------------------------
/**
 * Get the number of commands.
 */
int cli_cmds_get_num(cli_cmds_t * cmds)
{
  return ptr_array_length(cmds);
}

// -----[ _cli_cmds_set_parent ]-------------------------------------
/**
 * Set the parent field of all the commands in this set.
 */
static inline void _cli_cmds_set_parent(cli_cmds_t * cmds,
					cli_cmd_t * parent)
{
  unsigned int index;

  for (index= 0; index < cli_cmds_get_num(cmds); index++)
    ((cli_cmd_t *) cmds->data[index])->parent= parent;
}

// ----- cli_matching_cmds ------------------------------------------
/**
 * Create a sublist of the commands included in the list (pCmds). Only
 * the commands that match the given text (pcText) are included.
 *
 * IMPORTANT NOTE: the returned list is not a classical command list,
 * since the commands that it includes are only references to the
 * commands from the original list. However, the returned list of
 * commands can be freed using the cli_cmds_destroy function, since
 * the underlying array's destroy function is NULL.
 */
cli_cmds_t * cli_matching_cmds(cli_cmds_t * cmds, const char * text)
{
  unsigned int index;
  int text_len= strlen(text);
  cli_cmds_t * matching_cmds=
    (cli_cmds_t *) ptr_array_create(ARRAY_OPTION_SORTED,
				    _cli_cmds_item_compare,
				    NULL, NULL);

  if (cmds == NULL) {
    return matching_cmds;
  }
  for (index= 0; index < ptr_array_length(cmds); index++) {
    if (strncmp(text, ((cli_cmd_t *) cmds->data[index])->name,
		text_len) == 0) {
      ptr_array_add(matching_cmds, &cmds->data[index]);
    }
  }
  return matching_cmds;
}


// ----- cli_cmd_find_submd -----------------------------------------
/**
 *
 */
cli_cmd_t * cli_cmd_find_subcmd(cli_cmd_t * cmd, char * name)
{
  char ** x= &name;
  unsigned int index;

  if (cmd->sub_cmds != NULL)
    if (!ptr_array_sorted_find_index(cmd->sub_cmds, &x, &index))
      return cmd->sub_cmds->data[index];
  return NULL;
}

// ----- cli_cmd_get_num_subcmds ------------------------------------
/**
 *
 */
int cli_cmd_get_num_subcmds(cli_cmd_t * cmd)
{
  if (cmd->sub_cmds != NULL)
    return ptr_array_length(cmd->sub_cmds);
  return 0;
}

// ----- cli_cmd_get_subcmd_at --------------------------------------
/**
 *
 */
cli_cmd_t * cli_cmd_get_subcmd_at(cli_cmd_t * cmd, unsigned int index)
{
  return (cli_cmd_t *) cmd->sub_cmds->data[index];
}

// ----- cli_cmd_add_subcmd -----------------------------------------
/**
 *
 */
int cli_cmd_add_subcmd(cli_cmd_t * cmd, cli_cmd_t * sub_cmd)
{
  if (cmd->sub_cmds == NULL)
    cmd->sub_cmds= cli_cmds_create();
  sub_cmd->parent= cmd;
  return cli_cmds_add(cmd->sub_cmds, sub_cmd);
}

// ----- cli_cmd_add_param ------------------------------------------
/**
 *
 */
int cli_cmd_add_param(cli_cmd_t * cmd, char * name,
		      FCliCheckParam fCheck)
{
  if (cmd->params == NULL)
    cmd->params= cli_params_create();
  return cli_params_add(cmd->params, name, fCheck);
}

// ----- cli_cmd_get_num_params -------------------------------------
/**
 *
 */
int cli_cmd_get_num_params(cli_cmd_t * cmd)
{
  if (cmd->params == NULL)
    return 0;
  return ptr_array_length(cmd->params);
}

// ----- cli_cmd_get_param_at ---------------------------------------
/**
 *
 */
cli_param_t * cli_cmd_get_param_at(cli_cmd_t * cmd, unsigned int index)
{
  return (cli_param_t *) cmd->params->data[index];
}

// ----- cli_cmd_add_option -----------------------------------------
/**
 *
 */
int cli_cmd_add_option(cli_cmd_t * cmd,
		       char * name,
		       FCliCheckParam fCheck)
{
  if (cmd->options == NULL)
    cmd->options= cli_options_create();
  return cli_options_add(cmd->options, name, fCheck);
}

// ----- _cli_cmd_match ---------------------------------------------
/**
 * Match the given string with the CLI command tree.
 *
 * Return values:
 *   CLI_MATCH_NOTHING      : could not match
 *   CLI_MATCH_COMMAND      : matched a command (command returned)
 *   CLI_MATCH_OPTION_NAMES : matched an option name (command returned)
 *   CLI_MATCH_OPTION_VALUE : matched an option value (option returned)
 *   CLI_MATCH_PARAM_VALUE  : matched a parameter value (parameter returned)
 */
int cli_cmd_match(cli_t * cli, cli_cmd_t * cmd, char * pcStartCmd,
		  char * pcEndCmd, void ** pctx)
{
  const gds_tokens_t * tokens;
  int iTokenIndex= 0;
  int iLastTokenIndex;
  int iParamIndex;
  char * pcToken;
  char * pcValue, * name;
  cli_option_t * option;
  int result;
  
  tokenizer_set_flag(cli->tokenizer, TOKENIZER_OPT_EMPTY_FINAL);
  result= tokenizer_run(cli->tokenizer, pcStartCmd);
  tokenizer_reset_flag(cli->tokenizer, TOKENIZER_OPT_EMPTY_FINAL);
  if (result < 0)
    return CLI_MATCH_NOTHING;
  
  tokens= tokenizer_get_tokens(cli->tokenizer);
  iLastTokenIndex= tokens_get_num(tokens)-1;
  
  // Match all tokens...
  while (iTokenIndex < tokens_get_num(tokens)) {
    pcToken= tokens_get_string_at(tokens, iTokenIndex);
    
    if ((iTokenIndex == iLastTokenIndex) &&
	!strcmp(pcToken, "")) {
      *pctx= cmd;
      return CLI_MATCH_COMMAND;
    }
    
    // Current token matches a sub-command ?
    cmd= cli_cmd_find_subcmd(cmd, pcToken);
    if (cmd == NULL) {
      *pctx= cmd;
      return CLI_MATCH_NOTHING;
    }
    iTokenIndex++;
    
    // Match options (if supported)...
    while (iTokenIndex < tokens_get_num(tokens)) {
      pcToken= tokens_get_string_at(tokens, iTokenIndex);
      
      // Is this an option ?
      if (strncmp(pcToken, "--", 2))
	break;
      pcToken+= 2;
      
      // Does the command support options ?
      if (cmd->options == NULL)
	return CLI_MATCH_NOTHING;
      
      // Locate option name/value
      pcValue= pcToken;
      name= strsep(&pcValue, "=");
      option= cli_options_find(cmd->options, pcToken);
      if (option == NULL)
	return CLI_MATCH_NOTHING;
      
      if (pcValue != NULL) {
	// If we have received the name, but no value and this is the
	// last token, need to complete value
	if (iTokenIndex == iLastTokenIndex) {
	  *pctx= option;
	  return CLI_MATCH_OPTION_VALUE;
	}
      }
      
      iTokenIndex++;
    }
    
    // Match params...
    iParamIndex= 0;
    while (iParamIndex < cli_cmd_get_num_params(cmd)) {
      if (iTokenIndex == iLastTokenIndex) {
	
	// Try to match an option ?
	if ((cmd->options != NULL) && (pcEndCmd != NULL) &&
	    (!strncmp(pcEndCmd, "--", 2))) {
	  *pctx= cmd;
	  return CLI_MATCH_OPTION_NAMES;
	}
	
	*pctx= cmd->params->data[iParamIndex];
	return CLI_MATCH_PARAM_VALUE;
      }
      
      iTokenIndex++;
      iParamIndex++;
    }
    
  }
  
  *pctx= cmd;
  return CLI_MATCH_COMMAND;
}

// ----- _cli_cmd_process_options -----------------------------------
/**
 * Process a token, check if it is a valid option, and store its
 * value in the command.
 *
 * Return value:
 *   - CLI_SUCCESS              in case of success
 *   - CLI_ERROR_UNKNOWN_OPTION if the option is not supported
 *   - CLI_ERROR_BAD_OPTION     if the option value was not valid
 *   - CLI_STATUS_NOT_AN_OPTION if the token is not an option
 */
static int _cli_cmd_process_options(cli_t * cli)
{
  char * pcValue;
  int result;
  cli_cmd_t * cmd= cli->ctx->cmd;
  const gds_tokens_t * tokens= tokenizer_get_tokens(cli->tokenizer);
  char * pcToken;
  char * name;

  while (cli->uExecTokenIndex < tokens_get_num(tokens)) {
    pcToken= tokens_get_string_at(tokens, cli->uExecTokenIndex);

    // Is this an option ?
    if (strncmp(pcToken, "--", 2))
      break;
    pcToken+= 2;
    
    // Does the command support options ?
    if (cmd->options == NULL)
      return CLI_ERROR_UNKNOWN_OPTION;
    
    // Locate option specification
    pcValue= pcToken;
    name= strsep(&pcValue, "=");
    result= cli_options_set_value(cmd->options, name, pcValue);
    if (result)
      return result;

    cli->uExecTokenIndex++;
  }
  
  return CLI_SUCCESS;
}

// ----- _cli_cmd_process_params ------------------------------------
/**
 * Process a token, check if it is a valid parameter, and store its
 * value in the command.
 *
 * Return value:
 *   - CLI_SUCCESS               in case of success
 *   - CLI_ERROR_MISSING_PARAM   if there is not enough parameter
 */
static int _cli_cmd_process_params(cli_t * cli)
{
  const gds_tokens_t * tokens= tokenizer_get_tokens(cli->tokenizer);
  char * pcToken;
  unsigned int uParamIndex= 0;
  cli_param_t * param;
  int result;
  cli_cmd_t * cmd= cli->ctx->cmd;
  unsigned int uNumTokens;

  // Does the command support parameters
  if (cmd->params == NULL)
    return CLI_SUCCESS;

  while (uParamIndex < cli_params_num(cmd->params)) {
    param= cli_cmd_get_param_at(cmd, uParamIndex);

    uNumTokens= tokens_get_num(tokens)-cli->uExecTokenIndex;

    // If the parameter is of type vararg (CLI_PARAM_TYPE_VARARG), it
    // will eat all the remaining arguments. In the other case, we
    // must check if there is still enough parameters.
    if (param->type == CLI_PARAM_TYPE_STD) {
      if (uNumTokens < 1)
	return CLI_ERROR_MISSING_PARAM;
      uNumTokens= 1;
    } else { /* pParam->tType == CLI_PARAM_TYPE_VARARG */
      if (uNumTokens > param->max_args)
	return CLI_ERROR_TOO_MANY_PARAMS;
    }

    // Parse each token for this parameter specification
    while (uNumTokens-- > 0) {
      pcToken= tokens_get_string_at(tokens, cli->uExecTokenIndex);

      // Check the parameter's value (if supported)
      if (param->fCheck != NULL) {
	result= param->fCheck(pcToken);
	if (result)
	  return result; // parameter error !!!
      }
      
      // Add the token to the list of values
      if (cmd->param_values == NULL)
	cmd->param_values= tokens_create();
      tokens_add_copy(cmd->param_values, pcToken);
      
      cli->uExecTokenIndex++;
    }
    uParamIndex++;
  }

  return CLI_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// COMMAND-LINE MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// ----- cli_create -------------------------------------------------
/**
 *
 */
cli_t * cli_create()
{
  cli_t * cli= (cli_t *) MALLOC(sizeof(cli_t));

  cli->tokenizer= tokenizer_create(CLI_DELIMITERS,
				   CLI_OPENING_QUOTES,
				   CLI_CLOSING_QUOTES);
  tokenizer_set_protect_quotes(cli->tokenizer, CLI_PROTECT_QUOTES);
  cli->root_cmd= NULL;
  cli->exec_param= NULL;
  cli->ctx= cli_context_create();
  cli->error.user_msg= NULL;
  return cli;
}

// ----- cli_destroy ------------------------------------------------
/**
 *
 */
void cli_destroy(cli_t ** pcli)
{
  if (*pcli != NULL) {
    tokenizer_destroy(&(*pcli)->tokenizer);
    cli_context_destroy(&(*pcli)->ctx);
    cli_cmd_destroy(&(*pcli)->root_cmd);
    if ((*pcli)->error.user_msg != NULL)
      free((*pcli)->error.user_msg);
    FREE(*pcli);
    *pcli= NULL;
  }
}

// -----[ cli_set_exit_callback ]------------------------------------
/**
 * Set the exit callback function. The default behaviour of the CLI
 * is to exit as soon as an error occurs. If this callback is
 * defined, it will be called in order to check it the CLI must
 * exit or not.
 */
void cli_set_exit_callback(cli_t * cli, FCliExitOnError fExitOnError)
{
  cli->fExitOnError= fExitOnError;
}

// -----[ cli_set_param_lookup ]-------------------------------------
void cli_set_param_lookup(cli_t * cli, param_lookup_t lookup)
{
  tokenizer_set_lookup(cli->tokenizer, lookup);
}

// ----- _cli_execute_cmd -------------------------------------------
/**
 * This function executes a CLI command.
 *
 * The function modifies the following state variable of the CLI:
 *   - exec_param
 *   - uExecTokenIndex
 *   - pExecContext: will push context on the stack if the current
 *                   command produces context
 */
static int _cli_execute_cmd(cli_t * cli, cli_cmd_t * cmd)
{
  int result;
  const gds_tokens_t * tokens= tokenizer_get_tokens(cli->tokenizer);

  cli_set_user_error(cli, NULL);

  cli->ctx->cmd= cmd;
  // Init command fields (param values and options)
  _cli_cmd_init(cmd);

  // STEP (1): MATCH ALL OPTIONS
  if ((result= _cli_cmd_process_options(cli)) != CLI_SUCCESS)
    return result;

  // STEP (2): MATCH ALL PARAMETERS
  if ((result= _cli_cmd_process_params(cli)) != CLI_SUCCESS)
    return result;

  // --------------------------------------------
  // STEP (3): PRODUCE CONTEXT / EXECUTE COMMAND ?
  // --------------------------------------------
  if (cli->ctx->cmd->fCtxCreate != NULL) {
    if ((result= cli->ctx->cmd->fCtxCreate(cli->ctx,
					       &cli->ctx->user_data))
	!= CLI_SUCCESS)
      return result;
    cli_context_push(cli->ctx);
  } else if (cli->uExecTokenIndex == tokens_get_num(tokens)) {
    if (cli->ctx->cmd->fCommand != NULL) {
      if ((result= cli->ctx->cmd->fCommand(cli->ctx, cli->ctx->cmd))
	  != CLI_SUCCESS)
	return result;
      cli_context_restore_depth(cli->ctx);
    } else
      return CLI_ERROR_NOT_A_COMMAND;
  }
      
  return CLI_SUCCESS;
}

// ----- cli_execute_ctx -------------------------------------------
/**
 * Execute the given command (pcCmd) with the given CLI (cli) and
 * the given context (user-data).
 */
int cli_execute_ctx(cli_t * cli, const char * cmd, void * user_data)
{
  int result= CLI_SUCCESS;
  int result2;
  const gds_tokens_t * tokens;
  char * pcToken;
  cli_cmd_t * pNewCmd;
  _cli_ctx_item_t * ctx_item;

  cli->ctx->user_data= user_data;

  // --------------------------------------------
  // STEP (1): Split command line into tokens...
  // --------------------------------------------
  result2= tokenizer_run(cli->tokenizer, cmd);
  if (result2 < 0) {
    cli_set_user_error(cli, tokenizer_strerror(result2));
    return CLI_ERROR_SYNTAX;
  }
  tokens= tokenizer_get_tokens(cli->tokenizer);


  // --------------------------------------------
  // STEP (2): Handle special commands
  // --------------------------------------------
  // Empty line ?
  if (tokens_get_num(tokens) == 0) {
    cli_context_clear(cli->ctx);
    return CLI_SUCCESS;
  }
  // Single "exit" token ?
  if ((tokens_get_num(tokens) == 1) &&
      (!strcmp("exit", tokens_get_string_at(tokens, 0)))) {
    cli_context_pop(cli->ctx);
    return CLI_SUCCESS;
  }
  // Ends with "?" token ?
  if (!strcmp("?", tokens_get_string_at(tokens, tokens_get_num(tokens)-1))) {
    return CLI_SUCCESS_HELP;
  }

  
  // --------------------------------------------
  // STEP (3): Retrieve current context
  // --------------------------------------------
  if (!cli_context_is_empty(cli->ctx)) {
    ctx_item= cli_context_top(cli->ctx);
    assert(ctx_item != NULL);
    cli->ctx->cmd= ctx_item->cmd;
  } else 
    cli->ctx->cmd= cli->root_cmd;

  
  // --------------------------------------------
  // STEP (4): Match tokens with command...
  // --------------------------------------------
  cli->uExecTokenIndex= 0;
  cli->exec_param= NULL;
  cli_context_save_depth(cli->ctx);
  while (cli->uExecTokenIndex < tokenizer_get_num_tokens(cli->tokenizer)) {
    if (cli->ctx->cmd != NULL) {
      pcToken= tokens_get_string_at(tokens, cli->uExecTokenIndex);
      pNewCmd= cli_cmd_find_subcmd(cli->ctx->cmd, pcToken);
      if (pNewCmd == NULL) {
	result= CLI_ERROR_UNKNOWN_COMMAND;
	break;
      }
      cli->ctx->cmd= pNewCmd;
      cli->uExecTokenIndex++;
      result= _cli_execute_cmd(cli, cli->ctx->cmd);
      if (result)
	break;
    } else {
      result= CLI_ERROR_UNKNOWN_COMMAND;
      break;
    }
  }

  // In case of error, restore saved context
  if (result < 0)
    cli_context_restore_depth(cli->ctx);

  return result;
}

// ----- cli_execute ------------------------------------------------
/**
 *
 */
int cli_execute(cli_t * cli, const char * cmd)
{
  cli->error.error= cli_execute_ctx(cli, cmd, NULL);
  return cli->error.error;
}

// ----- cli_register_cmd -------------------------------------------
/**
 *
 */
int cli_register_cmd(cli_t * cli, cli_cmd_t * cmd)
{
  if (cli->root_cmd == NULL)
    cli->root_cmd= cli_cmd_create("", NULL, NULL, NULL);
  return cli_cmd_add_subcmd(cli->root_cmd, cmd);
}

// ----- cli_perror -------------------------------------------------
/**
 *
 */
void cli_perror(gds_stream_t * stream, int error)
{
  const char * error_str= cli_strerror(error);
  if (error_str != NULL)
    stream_printf(stream, error_str);
  else
    stream_printf(stream, "unknown error (%i)", error);
}

// ----- cli_strerror -----------------------------------------------
/**
 *
 */
const char * cli_strerror(int error)
{
  switch (error) {
  case CLI_SUCCESS:
    return "success";
  case CLI_ERROR_GENERIC:
    return "error";
  case CLI_ERROR_UNEXPECTED:
    return "unexpected error";
  case CLI_ERROR_UNKNOWN_COMMAND:
    return "unknown command";
  case CLI_ERROR_MISSING_PARAM:
    return "missing parameter";
  case CLI_ERROR_TOO_MANY_PARAMS:
    return "too many parameters (vararg)";
  case CLI_ERROR_NOT_A_COMMAND:
    return "not a command";
  case CLI_ERROR_COMMAND_FAILED:
    return "command failed";
  case CLI_ERROR_BAD_PARAM:
    return "bad parameter value";
  case CLI_ERROR_CTX_CREATE:
    return "unable to create context";
  case CLI_ERROR_UNKNOWN_OPTION:
    return "unknown option";
  case CLI_ERROR_BAD_OPTION:
    return "bad option value";
  case CLI_WARNING_EMPTY_COMMAND:
    return "empty command";
  case CLI_ERROR_SYNTAX:
    return "syntax error";
  }
  return NULL;
}

// ----- cli_perror_details -----------------------------------------
/**
 *
 */
void cli_perror_details(gds_stream_t * stream, int result, cli_t * cli,
			const char * line)
{
  const gds_tokens_t * tokens;
  int index;

  stream_printf(stream, "*** command: \"%s\"\n", line);
  if ((result == CLI_ERROR_UNKNOWN_COMMAND) ||
      (result == CLI_ERROR_NOT_A_COMMAND) ||
      (result == CLI_ERROR_MISSING_PARAM) ||
      (result == CLI_ERROR_BAD_PARAM) ||
      (result == CLI_ERROR_UNKNOWN_OPTION) ||
      (result == CLI_ERROR_BAD_OPTION)) {
    stream_printf(stream, "*** error  : \"");
    tokens= tokenizer_get_tokens(cli->tokenizer);
    for (index= 0; index < cli->uExecTokenIndex; index++)
      stream_printf(stream, "%s ", tokens_get_string_at(tokens, index));
    stream_printf(stream, "^^^\"\n");

    switch (result) {
    case CLI_ERROR_UNKNOWN_COMMAND:
    case CLI_ERROR_NOT_A_COMMAND:
      if (cli->ctx->cmd != NULL) {
	stream_printf(stream, "*** expect : ");

	for (index= 0;
	     index < cli_cmd_get_num_subcmds(cli->ctx->cmd);
	     index++) {
	  if (index > 0)
	    stream_printf(stream, ", ");
	  stream_printf(stream, "%s",
			cli_cmd_get_subcmd_at(cli->ctx->cmd, index)->name);
	}
	stream_printf(stream, "\n");
      }
      break;

    case CLI_ERROR_MISSING_PARAM:
    case CLI_ERROR_BAD_PARAM:
      if (cli->exec_param != NULL)
	stream_printf(stream, "*** expect : %s\n", cli->exec_param->name);
      break;

    case CLI_ERROR_UNKNOWN_OPTION:
    case CLI_ERROR_BAD_OPTION:
      //stream_printf(stream, "*** option : %s\n", cli->pcOption);
      break;
      
    }
  }
}

// ----- cli_execute_line -------------------------------------------
/**
 *
 */
int cli_execute_line(cli_t * cli, const char * line)
{
  int result= CLI_SUCCESS;

  // Skip leading blank spaces
  while ((*line == ' ') || (*line == '\t'))
    line++;

  // Skip commented lines
  if (*line == '#')
    return result;

  // Parse and execute command
  result= cli_execute(cli, (char *) line);
  if (result < 0) {
    if (cli->error.user_msg != NULL)
      stream_printf(gdserr, "Error: %s\n", cli->error.user_msg);
    stream_printf(gdserr, "\033[0;31;1mError: ");
    cli_perror(gdserr, result);
    stream_printf(gdserr, "\033[0m\n");
    cli_perror_details(gdserr, result, cli, line);
  }
  return result;
}

// ----- cli_execute_file -------------------------------------------
/**
 * Execute all the lines from the given input stream.
 *
 * NOTE: The execution of the script will stop at the first error.
 */
int cli_execute_file(cli_t * cli, FILE * stream)
{
  int iLen;
  char acLine[MAX_CLI_LINE_LENGTH];
  int result;
  cli->error.line_number= 1;

  /* Execute all lines in the input file... */
  while (fgets(acLine, sizeof(acLine), stream) != NULL) {
    /* Chop trailing '\n' */
    iLen= strlen(acLine);
    if ((iLen >= 1) && (acLine[iLen-1] == '\n'))
      acLine[iLen-1]= '\0';

    result= cli_execute_line(cli, acLine);
    if (result < 0) {
      stream_printf(gdserr, "*** in script file, line %d\n",
		    cli->error.line_number);

      // In case of error, we call the exit-on-error function (if
      // it is defined). The decision to stop depends on the error
      // code returned by this function. If the fExitOnError function
      // is not defined, we just return the error code.
      if (cli->fExitOnError != NULL)
	result= cli->fExitOnError(result);
      if (result)
	return result;

    }
    cli->error.line_number++;
  }

  /* Clear the context (if required) */
  cli_context_clear(cli->ctx);

  return CLI_SUCCESS;
}

// ----- cli_get_cmd_context --------------------------------------
/**
 * Returns the current command context. If there is currently no
 * context, the root command is returned.
 */
cli_cmd_t * cli_get_cmd_context(cli_t * cli)
{
  cli_cmd_t * pCmdContext= NULL;
  _cli_ctx_item_t * ctx_item;

  // Get current command from context.
  if (cli->ctx != NULL) {
    ctx_item= cli_context_top(cli->ctx);
    if (ctx_item != NULL)
      pCmdContext= ctx_item->cmd;
  }

  // If no current command, return root command.
  if (pCmdContext == NULL)
    pCmdContext= cli->root_cmd;

  return pCmdContext;
}

// ----- cli_get_error_details --------------------------------------
int cli_get_error_details(cli_t * cli, cli_error_t * error)
{
  memcpy(error, &cli->error, sizeof(cli_error_t));
  return cli->error.error;
}

// ----- cli_set_user_error -----------------------------------------
void cli_set_user_error(cli_t * cli, const char * format, ...)
{
#define CLI_USER_ERROR_BUFFER_SIZE 1000
  char acBuffer[CLI_USER_ERROR_BUFFER_SIZE];
  va_list ap;

  if (cli->error.user_msg != NULL) {
    free(cli->error.user_msg);
    cli->error.user_msg= NULL;
  }

  if (format != NULL) {
    va_start(ap, format);
    vsnprintf(acBuffer, CLI_USER_ERROR_BUFFER_SIZE, format, ap);
    
    cli->error.user_msg= strdup(acBuffer);
  }
}
