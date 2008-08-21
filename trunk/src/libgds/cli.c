// ==================================================================
// @(#)cli.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 25/06/2003
// @lastdate 03/07/2008
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <libgds/cli.h>
#include <libgds/cli_ctx.h>
#include <libgds/log.h>
#include <libgds/memory.h>
#include <libgds/str_util.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define CLI_DELIMITERS " \t"
#define CLI_OPENING_QUOTES "\""
#define CLI_CLOSING_QUOTES "\""

static inline void _cli_cmds_set_parent(SCliCmds * pCmds,
					SCliCmd * pParent);

/////////////////////////////////////////////////////////////////////
//
// COMMANDS MANAGMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// ----- _cli_cmd_init ----------------------------------------------
/**
 * Initialize the parameters and option values of the command.
 */
static void _cli_cmd_init(SCliCmd * pCmd)
{
  tokens_destroy(&pCmd->pParamValues);
  if (pCmd->pOptions != NULL)
    cli_options_init(pCmd->pOptions);
}

// -----[ _cli_cmd_create ]------------------------------------------
static SCliCmd * _cli_cmd_create(char * pcName,
				 FCliCommand fCommand,
				 FCliContextCreate fCtxCreate,
				 FCliContextDestroy fCtxDestroy,
				 SCliCmds * pSubCmds,
				 SCliParams * pParams)
{
  SCliCmd * pCmd= (SCliCmd *) MALLOC(sizeof(SCliCmd));

  pCmd->pcName= (char *) MALLOC(sizeof(char)*(strlen(pcName)+1));
  strcpy(pCmd->pcName, pcName);
  pCmd->pOptions= NULL;
  pCmd->pParams= pParams;
  pCmd->pSubCmds= pSubCmds;

  pCmd->pParent= NULL;
  pCmd->fCtxCreate= fCtxCreate;
  pCmd->fCtxDestroy= fCtxDestroy;
  pCmd->fCommand= fCommand;
  pCmd->pcHelp= NULL;
  pCmd->pParamValues= NULL;

  if (pSubCmds != NULL)
    _cli_cmds_set_parent(pSubCmds, pCmd);
  return pCmd;
}

// ----- cli_cmd_create ---------------------------------------------
/**
 * Create a CLI command (executable).
 */
SCliCmd * cli_cmd_create(char * pcName,
			 FCliCommand fCommand,
			 SCliCmds * pSubCmds,
			 SCliParams * pParams)
{
  return _cli_cmd_create(pcName,
			 fCommand,
			 NULL, NULL,
			 pSubCmds, pParams);
}

// ----- cli_cmd_create_ctx -----------------------------------------
/**
 * Create a CLI context command (not executable).
 */
SCliCmd * cli_cmd_create_ctx(char * pcName,
			     FCliContextCreate fCtxCreate,
			     FCliContextDestroy fCtxDestroy,
			     SCliCmds * pSubCmds,
			     SCliParams * pParams)
{
  return _cli_cmd_create(pcName,
			 NULL,
			 fCtxCreate, fCtxDestroy,
			 pSubCmds, pParams);
}

// ----- cli_cmd_destroy --------------------------------------------
/**
 * Destroy a CLI command.
 */
void cli_cmd_destroy(SCliCmd ** ppCmd)
{
  if (*ppCmd != NULL) {
    FREE((*ppCmd)->pcName);
    ptr_array_destroy(&(*ppCmd)->pSubCmds);
    cli_params_destroy(&(*ppCmd)->pParams);
    cli_options_destroy(&(*ppCmd)->pOptions);
    tokens_destroy(&(*ppCmd)->pParamValues);
    FREE(*ppCmd);
    *ppCmd= NULL;
  }
}

// ----- cli_cmd_dump -----------------------------------------------
/**
 *
 */
void cli_cmd_dump(SLogStream * pStream, char * pcPrefix, SCliCmd * pCmd)
{
  int iIndex;
  SCliCmd * pSubCmd;
  SCliOption * pOption;
  SCliParam * pParam;
  char * pcNewPrefix;

  log_printf(pStream, "%s%s", pcPrefix, pCmd->pcName);
  if (pCmd->pOptions != NULL) {
    for (iIndex= 0; iIndex < ptr_array_length(pCmd->pOptions); iIndex++) {
      pOption= (SCliOption *) pCmd->pOptions->data[iIndex];
      log_printf(pStream, " [--%s]", pOption->pcName);
    }
  }
  if (pCmd->pParams != NULL)
    for (iIndex= 0; iIndex < ptr_array_length(pCmd->pParams); iIndex++) {
      pParam= (SCliParam *) pCmd->pParams->data[iIndex];
      log_printf(pStream, " %s", pParam->pcName);
      if (pParam->tType == CLI_PARAM_TYPE_VARARG) {
	if (pParam->uMaxArgs > 0)
	  log_printf(pStream, "?(0-%d)", pParam->uMaxArgs);
	else
	  log_printf(pStream, "?(0-any)");
      }
    }
  log_printf(pStream, "\n");
  if (pCmd->pSubCmds != NULL) {
    pcNewPrefix= (char *) MALLOC(sizeof(char)*(strlen(pcPrefix)+3));
    strcpy(pcNewPrefix, pcPrefix);
    strcat(pcNewPrefix, "  ");
    for (iIndex= 0; iIndex < ptr_array_length(pCmd->pSubCmds); iIndex++) {
      pSubCmd= (SCliCmd *) pCmd->pSubCmds->data[iIndex];
      cli_cmd_dump(pStream, pcNewPrefix, pSubCmd);
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
static int _cli_cmds_item_compare(void * pItem1, void * pItem2,
				  unsigned int EltSize)
{
  SCliCmd * pCmd1= *((SCliCmd **) pItem1);
  SCliCmd * pCmd2= *((SCliCmd **) pItem2);

  return strcmp(pCmd1->pcName, pCmd2->pcName);
}

// ----- _cli_cmds_item_destroy -------------------------------------
/**
 * Free a command in a list of commands.
 */
static void _cli_cmds_item_destroy(void * pItem)
{
  cli_cmd_destroy((SCliCmd **) pItem);
}

// ----- cli_cmds_create --------------------------------------------
/**
 * Create a list of command. The commands in the list are sorted based
 * on the alphanumeric ordering (thanks to the _cli_cmds_item_compare
 * function).
 */
SCliCmds * cli_cmds_create()
{
  return (SCliCmds *) ptr_array_create(ARRAY_OPTION_SORTED,
				       _cli_cmds_item_compare,
				       _cli_cmds_item_destroy);
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
void cli_cmds_destroy(SCliCmds ** ppCmds)
{
  ptr_array_destroy((SPtrArray **) ppCmds);
}

// ----- cli_cmds_add -----------------------------------------------
/**
 * Add a command (pCmd) in the list (pCmds).
 */
int cli_cmds_add(SCliCmds * pCmds, SCliCmd * pCmd)
{
  return ptr_array_add((SPtrArray *) pCmds, &pCmd);
}

// -----[ cli_cmds_get_num ]-----------------------------------------
/**
 * Get the number of commands.
 */
int cli_cmds_get_num(SCliCmds * pCmds)
{
  return ptr_array_length(pCmds);
}

// -----[ _cli_cmds_set_parent ]-------------------------------------
/**
 * Set the parent field of all the commands in this set.
 */
static inline void _cli_cmds_set_parent(SCliCmds * pCmds,
					SCliCmd * pParent)
{
  unsigned int uIndex;

  for (uIndex= 0; uIndex < cli_cmds_get_num(pCmds); uIndex++)
    ((SCliCmd *) pCmds->data[uIndex])->pParent= pParent;
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
SCliCmds * cli_matching_cmds(SCliCmds * pCmds, const char * pcText)
{
  int iIndex;
  int iTextLen= strlen(pcText);
  SCliCmds * pMatchingCmds=
    (SCliCmds *) ptr_array_create(ARRAY_OPTION_SORTED,
				  _cli_cmds_item_compare, NULL);

  if (pCmds == NULL) {
    return pMatchingCmds;
  }
  for (iIndex= 0; iIndex < ptr_array_length(pCmds); iIndex++) {
    if (strncmp(pcText, ((SCliCmd *) pCmds->data[iIndex])->pcName,
		iTextLen) == 0) {
      ptr_array_add(pMatchingCmds, &pCmds->data[iIndex]);
    }
  }
  return pMatchingCmds;
}


// ----- cli_cmd_find_submd -----------------------------------------
/**
 *
 */
SCliCmd * cli_cmd_find_subcmd(SCliCmd * pCmd, char * pcName)
{
  char ** x= &pcName;
  unsigned int uIndex;

  if (pCmd->pSubCmds != NULL)
    if (!ptr_array_sorted_find_index(pCmd->pSubCmds, &x, &uIndex))
      return pCmd->pSubCmds->data[uIndex];
  return NULL;
}

// ----- cli_cmd_get_num_subcmds ------------------------------------
/**
 *
 */
int cli_cmd_get_num_subcmds(SCliCmd * pCmd)
{
  if (pCmd->pSubCmds != NULL)
    return ptr_array_length(pCmd->pSubCmds);
  return 0;
}

// ----- cli_cmd_get_subcmd_at --------------------------------------
/**
 *
 */
SCliCmd * cli_cmd_get_subcmd_at(SCliCmd * pCmd, int iIndex)
{
  return (SCliCmd *) pCmd->pSubCmds->data[iIndex];
}

// ----- cli_cmd_add_subcmd -----------------------------------------
/**
 *
 */
int cli_cmd_add_subcmd(SCliCmd * pCmd, SCliCmd * pSubCmd)
{
  if (pCmd->pSubCmds == NULL)
    pCmd->pSubCmds= cli_cmds_create();
  pSubCmd->pParent= pCmd;
  return cli_cmds_add(pCmd->pSubCmds, pSubCmd);
}

// ----- cli_cmd_add_param ------------------------------------------
/**
 *
 */
int cli_cmd_add_param(SCliCmd * pCmd, char * pcName,
		      FCliCheckParam fCheck)
{
  if (pCmd->pParams == NULL)
    pCmd->pParams= cli_params_create();
  return cli_params_add(pCmd->pParams, pcName, fCheck);
}

// ----- cli_cmd_get_num_params -------------------------------------
/**
 *
 */
int cli_cmd_get_num_params(SCliCmd * pCmd)
{
  if (pCmd->pParams == NULL)
    return 0;
  return ptr_array_length(pCmd->pParams);
}

// ----- cli_cmd_get_param_at ---------------------------------------
/**
 *
 */
SCliParam * cli_cmd_get_param_at(SCliCmd * pCmd, uint32_t uIndex)
{
  return (SCliParam *) pCmd->pParams->data[uIndex];
}

// ----- cli_cmd_add_option -----------------------------------------
/**
 *
 */
int cli_cmd_add_option(SCliCmd * pCmd,
		       char * pcName,
		       FCliCheckParam fCheck)
{
  if (pCmd->pOptions == NULL)
    pCmd->pOptions= cli_options_create();
  return cli_options_add(pCmd->pOptions, pcName, fCheck);
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
int cli_cmd_match(SCli * pCli, SCliCmd * pCmd, char * pcStartCmd,
		  char * pcEndCmd, void ** ppCtx)
{
  STokens * pTokens;
  int iTokenIndex= 0;
  int iLastTokenIndex;
  int iParamIndex;
  char * pcToken;
  char * pcValue, * pcName;
  SCliOption * pOption;
  int iResult;
  
  pCli->pTokenizer->iAllowFinalEmptyField= 1;
  iResult= tokenizer_run(pCli->pTokenizer, pcStartCmd);
  pCli->pTokenizer->iAllowFinalEmptyField= 0;
  if (iResult < 0)
    return CLI_MATCH_NOTHING;
  
  pTokens= tokenizer_get_tokens(pCli->pTokenizer);
  iLastTokenIndex= tokens_get_num(pTokens)-1;
  
  // Match all tokens...
  while (iTokenIndex < tokens_get_num(pTokens)) {
    pcToken= tokens_get_string_at(pTokens, iTokenIndex);
    
    if ((iTokenIndex == iLastTokenIndex) &&
	!strcmp(pcToken, "")) {
      *ppCtx= pCmd;
      return CLI_MATCH_COMMAND;
    }
    
    // Current token matches a sub-command ?
    pCmd= cli_cmd_find_subcmd(pCmd, pcToken);
    if (pCmd == NULL) {
      *ppCtx= pCmd;
      return CLI_MATCH_NOTHING;
    }
    iTokenIndex++;
    
    // Match options (if supported)...
    while (iTokenIndex < tokens_get_num(pTokens)) {
      pcToken= tokens_get_string_at(pTokens, iTokenIndex);
      
      // Is this an option ?
      if (strncmp(pcToken, "--", 2))
	break;
      pcToken+= 2;
      
      // Does the command support options ?
      if (pCmd->pOptions == NULL)
	return CLI_MATCH_NOTHING;
      
      // Locate option name/value
      pcValue= pcToken;
      pcName= strsep(&pcValue, "=");
      pOption= cli_options_find(pCmd->pOptions, pcToken);
      if (pOption == NULL)
	return CLI_MATCH_NOTHING;
      
      if (pcValue != NULL) {
	// If we have received the name, but no value and this is the
	// last token, need to complete value
	if (iTokenIndex == iLastTokenIndex) {
	  *ppCtx= pOption;
	  return CLI_MATCH_OPTION_VALUE;
	}
      }
      
      iTokenIndex++;
    }
    
    // Match params...
    iParamIndex= 0;
    while (iParamIndex < cli_cmd_get_num_params(pCmd)) {
      if (iTokenIndex == iLastTokenIndex) {
	
	// Try to match an option ?
	if ((pCmd->pOptions != NULL) && (pcEndCmd != NULL) &&
	    (!strncmp(pcEndCmd, "--", 2))) {
	  *ppCtx= pCmd;
	  return CLI_MATCH_OPTION_NAMES;
	}
	
	*ppCtx= pCmd->pParams->data[iParamIndex];
	return CLI_MATCH_PARAM_VALUE;
      }
      
      iTokenIndex++;
      iParamIndex++;
    }
    
  }
  
  *ppCtx= pCmd;
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
static int _cli_cmd_process_options(SCli * pCli)
{
  char * pcValue;
  int iResult;
  SCliCmd * pCmd= pCli->pCtx->pCmd;
  STokens * pTokens= tokenizer_get_tokens(pCli->pTokenizer);
  char * pcToken;
  char * pcName;

  while (pCli->uExecTokenIndex < tokens_get_num(pTokens)) {
    pcToken= tokens_get_string_at(pTokens, pCli->uExecTokenIndex);

    // Is this an option ?
    if (strncmp(pcToken, "--", 2))
      break;
    pcToken+= 2;
    
    // Does the command support options ?
    if (pCmd->pOptions == NULL)
      return CLI_ERROR_UNKNOWN_OPTION;
    
    // Locate option specification
    pcValue= pcToken;
    pcName= strsep(&pcValue, "=");
    iResult= cli_options_set_value(pCmd->pOptions, pcName, pcValue);
    if (iResult)
      return iResult;

    pCli->uExecTokenIndex++;
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
static int _cli_cmd_process_params(SCli * pCli)
{
  STokens * pTokens= tokenizer_get_tokens(pCli->pTokenizer);
  char * pcToken;
  unsigned int uParamIndex= 0;
  SCliParam * pParam;
  int iResult;
  SCliCmd * pCmd= pCli->pCtx->pCmd;
  unsigned int uNumTokens;

  // Does the command support parameters
  if (pCmd->pParams == NULL)
    return CLI_SUCCESS;

  while (uParamIndex < cli_params_num(pCmd->pParams)) {
    pParam= cli_cmd_get_param_at(pCmd, uParamIndex);

    uNumTokens= tokens_get_num(pTokens)-pCli->uExecTokenIndex;

    // If the parameter is of type vararg (CLI_PARAM_TYPE_VARARG), it
    // will eat all the remaining arguments. In the other case, we
    // must check if there is still enough parameters.
    if (pParam->tType == CLI_PARAM_TYPE_STD) {
      if (uNumTokens < 1)
	return CLI_ERROR_MISSING_PARAM;
      uNumTokens= 1;
    } else { /* pParam->tType == CLI_PARAM_TYPE_VARARG */
      if (uNumTokens > pParam->uMaxArgs)
	return CLI_ERROR_TOO_MANY_PARAMS;
    }

    // Parse each token for this parameter specification
    while (uNumTokens-- > 0) {
      pcToken= tokens_get_string_at(pTokens, pCli->uExecTokenIndex);

      // Check the parameter's value (if supported)
      if (pParam->fCheck != NULL) {
	iResult= pParam->fCheck(pcToken);
	if (iResult)
	  return iResult; // parameter error !!!
      }
      
      // Add the token to the list of values
      if (pCmd->pParamValues == NULL)
	pCmd->pParamValues= tokens_create();
      tokens_add_copy(pCmd->pParamValues, pcToken);
      
      pCli->uExecTokenIndex++;
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
SCli * cli_create()
{
  SCli * pCli= (SCli *) MALLOC(sizeof(SCli));

  pCli->pTokenizer= tokenizer_create(CLI_DELIMITERS,
				     0,
				     CLI_OPENING_QUOTES,
				     CLI_CLOSING_QUOTES);
  pCli->pBaseCommand= NULL;
  pCli->pExecParam= NULL;
  pCli->pCtx= cli_context_create();
  pCli->sErrorDetails.pcUserError= NULL;
  return pCli;
}

// ----- cli_destroy ------------------------------------------------
/**
 *
 */
void cli_destroy(SCli ** ppCli)
{
  if (*ppCli != NULL) {
    tokenizer_destroy(&(*ppCli)->pTokenizer);
    cli_context_destroy(&(*ppCli)->pCtx);
    cli_cmd_destroy(&(*ppCli)->pBaseCommand);
    if ((*ppCli)->sErrorDetails.pcUserError != NULL)
      free((*ppCli)->sErrorDetails.pcUserError);
    FREE(*ppCli);
    *ppCli= NULL;
  }
}

// ----- cli_set_exit_callback ------------------------------------
/**
 * Set the exit callback function. The default behaviour of the CLI
 * is to exit as soon as an error occurs. If this callback is
 * defined, it will be called in order to check it the CLI must
 * exit or not.
 */
void cli_set_exit_callback(SCli * pCli, FCliExitOnError fExitOnError)
{
  pCli->fExitOnError= fExitOnError;
}

// ----- _cli_execute_cmd -------------------------------------------
/**
 * This function executes a CLI command.
 *
 * The function modifies the following state variable of the CLI:
 *   - pExecParam
 *   - uExecTokenIndex
 *   - pExecContext: will push context on the stack if the current
 *                   command produces context
 */
static int _cli_execute_cmd(SCli * pCli, SCliCmd * pCmd)
{
  int iResult;
  STokens * pTokens= tokenizer_get_tokens(pCli->pTokenizer);

  cli_set_user_error(pCli, NULL);

  pCli->pCtx->pCmd= pCmd;
  // Init command fields (param values and options)
  _cli_cmd_init(pCmd);

  // STEP (1): MATCH ALL OPTIONS
  if ((iResult= _cli_cmd_process_options(pCli)) != CLI_SUCCESS)
    return iResult;

  // STEP (2): MATCH ALL PARAMETERS
  if ((iResult= _cli_cmd_process_params(pCli)) != CLI_SUCCESS)
    return iResult;

  // --------------------------------------------
  // STEP (3): PRODUCE CONTEXT / EXECUTE COMMAND ?
  // --------------------------------------------
  if (pCli->pCtx->pCmd->fCtxCreate != NULL) {
    if ((iResult= pCli->pCtx->pCmd->fCtxCreate(pCli->pCtx,
					       &pCli->pCtx->pUserData))
	!= CLI_SUCCESS)
      return iResult;
    cli_context_push(pCli->pCtx);
  } else if (pCli->uExecTokenIndex == tokens_get_num(pTokens)) {
    if (pCli->pCtx->pCmd->fCommand != NULL) {
      if ((iResult= pCli->pCtx->pCmd->fCommand(pCli->pCtx,
					       pCli->pCtx->pCmd))
	  != CLI_SUCCESS)
	return iResult;
      cli_context_restore_depth(pCli->pCtx);
    } else
      return CLI_ERROR_NOT_A_COMMAND;
  }
      
  return CLI_SUCCESS;
}

// ----- cli_execute_ctx -------------------------------------------
/**
 * Execute the given command (pcCmd) with the given CLI (pCli) and
 * the given context (user-data).
 */
int cli_execute_ctx(SCli * pCli, char * pcCmd, void * pUserData)
{
  int iResult= CLI_SUCCESS;
  STokens * pTokens;
  char * pcToken;
  SCliCmd * pNewCmd;
  SCliCtxItem * pCtxItem;

  pCli->pCtx->pUserData= pUserData;

  // --------------------------------------------
  // STEP (1): Split command line into tokens...
  // --------------------------------------------
  if (tokenizer_run(pCli->pTokenizer, pcCmd))
    return CLI_ERROR_UNEXPECTED;
  pTokens= tokenizer_get_tokens(pCli->pTokenizer);


  // --------------------------------------------
  // STEP (2): Handle special commands
  // --------------------------------------------
  // Empty line ?
  if (tokens_get_num(pTokens) == 0) {
    cli_context_clear(pCli->pCtx);
    return CLI_SUCCESS;
  }
  // Single "exit" token ?
  if ((tokens_get_num(pTokens) == 1) &&
      (!strcmp("exit", tokens_get_string_at(pTokens, 0)))) {
    cli_context_pop(pCli->pCtx);
    return CLI_SUCCESS;
  }
  // Ends with "?" token ?
  if (!strcmp("?", tokens_get_string_at(pTokens, tokens_get_num(pTokens)-1))) {
    return CLI_SUCCESS_HELP;
  }

  
  // --------------------------------------------
  // STEP (3): Retrieve current context
  // --------------------------------------------
  if (!cli_context_is_empty(pCli->pCtx)) {
    pCtxItem= cli_context_top(pCli->pCtx);
    assert(pCtxItem != NULL);
    pCli->pCtx->pCmd= pCtxItem->pCmd;
  } else 
    pCli->pCtx->pCmd= pCli->pBaseCommand;

  
  // --------------------------------------------
  // STEP (4): Match tokens with command...
  // --------------------------------------------
  pCli->uExecTokenIndex= 0;
  pCli->pExecParam= NULL;
  cli_context_save_depth(pCli->pCtx);
  while (pCli->uExecTokenIndex < tokenizer_get_num_tokens(pCli->pTokenizer)) {
    if (pCli->pCtx->pCmd != NULL) {
      pcToken= tokens_get_string_at(pTokens, pCli->uExecTokenIndex);
      pNewCmd= cli_cmd_find_subcmd(pCli->pCtx->pCmd, pcToken);
      if (pNewCmd == NULL) {
	iResult= CLI_ERROR_UNKNOWN_COMMAND;
	break;
      }
      pCli->pCtx->pCmd= pNewCmd;
      pCli->uExecTokenIndex++;
      iResult= _cli_execute_cmd(pCli, pCli->pCtx->pCmd);
      if (iResult)
	break;
    } else {
      iResult= CLI_ERROR_UNKNOWN_COMMAND;
      break;
    }
  }

  // In case of error, restore saved context
  if (iResult < 0)
    cli_context_restore_depth(pCli->pCtx);

  return iResult;
}

// ----- cli_execute ------------------------------------------------
/**
 *
 */
int cli_execute(SCli * pCli, char * pcCmd)
{
  pCli->sErrorDetails.iErrorCode= cli_execute_ctx(pCli, pcCmd, NULL);
  return pCli->sErrorDetails.iErrorCode;
}

// ----- cli_register_cmd -------------------------------------------
/**
 *
 */
int cli_register_cmd(SCli * pCli, SCliCmd * pCmd)
{
  if (pCli->pBaseCommand == NULL)
    pCli->pBaseCommand= cli_cmd_create("", NULL, NULL, NULL);
  return cli_cmd_add_subcmd(pCli->pBaseCommand, pCmd);
}

// ----- cli_perror -------------------------------------------------
/**
 *
 */
void cli_perror(SLogStream * pStream, int iErrorCode)
{
  char * pcErrorStr= cli_strerror(iErrorCode);
  if (pcErrorStr != NULL)
    log_printf(pStream, pcErrorStr);
  else
    log_printf(pStream, "unknown error (%i)", iErrorCode);
}

// ----- cli_strerror -----------------------------------------------
/**
 *
 */
char * cli_strerror(int iErrorCode)
{
  switch (iErrorCode) {
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
  }
  return NULL;
}

// ----- cli_perror_details -----------------------------------------
/**
 *
 */
void cli_perror_details(SLogStream * pStream, int iResult, SCli * pCli,
			const char * pcLine)
{
  STokens * pTokens;
  int iIndex;

  log_printf(pStream, "*** command: \"%s\"\n", pcLine);
  if ((iResult == CLI_ERROR_UNKNOWN_COMMAND) ||
      (iResult == CLI_ERROR_NOT_A_COMMAND) ||
      (iResult == CLI_ERROR_MISSING_PARAM) ||
      (iResult == CLI_ERROR_BAD_PARAM) ||
      (iResult == CLI_ERROR_UNKNOWN_OPTION) ||
      (iResult == CLI_ERROR_BAD_OPTION)) {
    log_printf(pStream, "*** error  : \"");
    pTokens= tokenizer_get_tokens(pCli->pTokenizer);
    for (iIndex= 0; iIndex < pCli->uExecTokenIndex; iIndex++)
      log_printf(pStream, "%s ", tokens_get_string_at(pTokens, iIndex));
    log_printf(pStream, "^^^\"\n");

    switch (iResult) {
    case CLI_ERROR_UNKNOWN_COMMAND:
    case CLI_ERROR_NOT_A_COMMAND:
      if (pCli->pCtx->pCmd != NULL) {
	log_printf(pStream, "*** expect : ");

	for (iIndex= 0;
	     iIndex < cli_cmd_get_num_subcmds(pCli->pCtx->pCmd);
	     iIndex++) {
	  if (iIndex > 0)
	    log_printf(pStream, ", ");
	  log_printf(pStream, "%s",
		     cli_cmd_get_subcmd_at(pCli->pCtx->pCmd, iIndex)->pcName);
	}
	log_printf(pStream, "\n");
      }
      break;

    case CLI_ERROR_MISSING_PARAM:
    case CLI_ERROR_BAD_PARAM:
      if (pCli->pExecParam != NULL) {
	log_printf(pStream, "*** expect : %s\n", pCli->pExecParam->pcName);
      }
      break;

    case CLI_ERROR_UNKNOWN_OPTION:
    case CLI_ERROR_BAD_OPTION:
      //log_printf(pStream, "*** option : %s\n", pCli->pcOption);
      break;
      
    }
  }
}

// ----- cli_execute_line -------------------------------------------
/**
 *
 */
int cli_execute_line(SCli * pCli, const char * pcLine)
{
  int iResult= CLI_SUCCESS;

  // Skip leading blank spaces
  while ((*pcLine == ' ') || (*pcLine == '\t'))
    pcLine++;

  // Skip commented lines
  if (*pcLine == '#')
    return iResult;

  // Parse and execute command
  iResult= cli_execute(pCli, (char *) pcLine);
  if (iResult < 0) {
    if (pCli->sErrorDetails.pcUserError != NULL)
      log_printf(pLogErr, "Error: %s\n", pCli->sErrorDetails.pcUserError);
    log_printf(pLogErr, "\033[0;31;1mError: ");
    cli_perror(pLogErr, iResult);
    log_printf(pLogErr, "\033[0m\n");
    cli_perror_details(pLogErr, iResult, pCli, pcLine);
  }
  return iResult;
}

// ----- cli_execute_file -------------------------------------------
/**
 * Execute all the lines from the given input stream.
 *
 * NOTE: The execution of the script will stop at the first error.
 */
int cli_execute_file(SCli * pCli, FILE * pStream)
{
  int iLen;
  char acLine[MAX_CLI_LINE_LENGTH];
  int iResult;
  pCli->sErrorDetails.iLineNumber= 1;

  /* Execute all lines in the input file... */
  while (fgets(acLine, sizeof(acLine), pStream) != NULL) {
    /* Chop trailing '\n' */
    iLen= strlen(acLine);
    if ((iLen >= 1) && (acLine[iLen-1] == '\n'))
      acLine[iLen-1]= '\0';

    iResult= cli_execute_line(pCli, acLine);
    if (iResult < 0) {
      log_printf(pLogErr, "*** in script file, line %d\n",
		 pCli->sErrorDetails.iLineNumber);

      // In case of error, we call the exit-on-error function (if
      // it is defined). The decision to stop depends on the error
      // code returned by this function. If the fExitOnError function
      // is not defined, we just return the error code.
      if (pCli->fExitOnError != NULL)
	iResult= pCli->fExitOnError(iResult);
      if (iResult)
	return iResult;

    }
    pCli->sErrorDetails.iLineNumber++;
  }

  /* Clear the context (if required) */
  cli_context_clear(pCli->pCtx);

  return CLI_SUCCESS;
}

// ----- cli_get_cmd_context --------------------------------------
/**
 * Returns the current command context. If there is currently no
 * context, the root command is returned.
 */
SCliCmd * cli_get_cmd_context(SCli * pCli)
{
  SCliCmd * pCmdContext= NULL;
  SCliCtxItem * pCtxItem;

  // Get current command from context.
  if (pCli->pCtx != NULL) {
    pCtxItem= cli_context_top(pCli->pCtx);
    if (pCtxItem != NULL)
      pCmdContext= pCtxItem->pCmd;
  }

  // If no current command, return root command.
  if (pCmdContext == NULL)
    pCmdContext= pCli->pBaseCommand;

  return pCmdContext;
}

// ----- cli_get_error_details --------------------------------------
int cli_get_error_details(SCli * pCli, SCliErrorDetails * psDetails)
{
  memcpy(psDetails, &pCli->sErrorDetails, sizeof(SCliErrorDetails));
  return pCli->sErrorDetails.iErrorCode;
}

// ----- cli_set_user_error -----------------------------------------
void cli_set_user_error(SCli * pCli, char * pcFormat, ...)
{
#define CLI_USER_ERROR_BUFFER_SIZE 1000
  char acBuffer[CLI_USER_ERROR_BUFFER_SIZE];
  va_list ap;

  if (pCli->sErrorDetails.pcUserError != NULL) {
    free(pCli->sErrorDetails.pcUserError);
    pCli->sErrorDetails.pcUserError= NULL;
  }

  if (pcFormat != NULL) {
    va_start(ap, pcFormat);
    vsnprintf(acBuffer, CLI_USER_ERROR_BUFFER_SIZE, pcFormat, ap);
    
    pCli->sErrorDetails.pcUserError= strdup(acBuffer);
  }
}
