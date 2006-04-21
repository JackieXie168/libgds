// ==================================================================
// @(#)cli.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 25/06/2003
// @lastdate 21/04/2006
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <libgds/cli.h>
#include <libgds/cli_ctx.h>
#include <libgds/log.h>
#include <libgds/memory.h>
#include <stdlib.h>
#include <string.h>

#define CLI_DELIMITERS " \t"
#define CLI_OPENING_QUOTES "\""
#define CLI_CLOSING_QUOTES "\""


/////////////////////////////////////////////////////////////////////
//
// PARAMETERS MANAGMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// ----- cli_cmd_param_create ---------------------------------------
/**
 * Create a parameter with default attributes.
 */
SCliCmdParam * cli_cmd_param_create(char * pcName,
				    FCliCheckParam fCheckParam)
{
  SCliCmdParam * pParam= (SCliCmdParam *) MALLOC(sizeof(SCliCmdParam));

  pParam->pcName= (char *) MALLOC(sizeof(char)*(strlen(pcName)+1));
  strcpy(pParam->pcName, pcName);
  pParam->fCheckParam= fCheckParam;
  pParam->fEnumParam= NULL;
  pParam->tType= CLI_PARAM_TYPE_STD;
  return pParam;
}

// ----- cli_cmd_param_destroy --------------------------------------
/**
 * Destroy a parameter.
 */
void cli_cmd_param_destroy(SCliCmdParam ** ppParam)
{
  if (*ppParam != NULL) {
    FREE((*ppParam)->pcName);
    FREE(*ppParam);
    *ppParam= NULL;
  }
}

// -----[ _cli_params_item_compare ----------------------------------
/**
 * Private helper function used to compare 2 parameters in a list.
 */
static int _cli_params_item_compare(void * pItem1, void * pItem2,
				    uint32_t uEltSize)
{
  SCliCmdParam * pParam1= *((SCliCmdParam **) pItem1);
  SCliCmdParam * pParam2= *((SCliCmdParam **) pItem2);

  return strcmp(pParam1->pcName, pParam2->pcName);
}

// -----[ _cli_params_item_destroy ]---------------------------------
/**
 * Private helper function used to destroy each parameter in a list.
 */
static void _cli_params_item_destroy(void * pItem)
{
  cli_cmd_param_destroy((SCliCmdParam **) pItem);
}

// ----- cli_params_create ------------------------------------------
/**
 * Create a list of parameters. A list of parameters is a sequence,
 * i.e. the ordering of parameters is the ordering of their insertion
 * in the list.
 */
SCliParams * cli_params_create()
{
  return (SCliParams *) ptr_array_create(0,
					 _cli_params_item_compare,
					 _cli_params_item_destroy);
}

// -----[ _cli_params_check_latest ]---------------------------------
/**
 * Check that the latest parameter on the list is not of type
 * CLI_PARAM_TYPE_VARARG. This function should be called before adding
 * any new parameter.
 */
static void _cli_params_check_latest(SCliParams * pParams)
{
  int iLength= ptr_array_length(pParams);
  if ((iLength > 0) &&
      (((SCliCmdParam *) pParams->data[iLength-1])->tType == CLI_PARAM_TYPE_VARARG)) {
    fprintf(stderr, "Error: can not add a parameter after a vararg parameter.\n");
    abort();
  }
}

// ----- cli_params_destroy -----------------------------------------
/**
 * Destroy a list of parameters.
 */
void cli_params_destroy(SCliParams ** ppParams)
{
  ptr_array_destroy((SPtrArray **) ppParams);
}

// ----- cli_params_add ---------------------------------------------
/**
 * Add a parameter to the list of parameters.
 *
 * Parameter type: CLI_PARAM_TYPE_STD
 */
int cli_params_add(SCliParams * pParams, char * pcName,
		   FCliCheckParam fCheckParam)
{
  SCliCmdParam * pParam= cli_cmd_param_create(pcName, fCheckParam);
  _cli_params_check_latest(pParams);
  return ptr_array_add((SPtrArray *) pParams, &pParam);
}

// ----- cli_params_add ---------------------------------------------
/**
 * Add a parameter to the list of parameters. An enumeration function
 * can be attached to this parameter. An enumeration function is
 * called by the command-line system in interactive mode.
 *
 * Parameter type: CLI_PARAM_TYPE_STD
 */
int cli_params_add2(SCliParams * pParams, char * pcName,
		    FCliCheckParam fCheckParam,
		    FCliEnumParam fEnumParam)
{
  SCliCmdParam * pParam= cli_cmd_param_create(pcName, fCheckParam);
  _cli_params_check_latest(pParams);
  pParam->fEnumParam= fEnumParam;
  return ptr_array_add((SPtrArray *) pParams, &pParam);
}

// ----- cli_params_add_vararg --------------------------------------
/**
 * Add a parameter that accepts a variable number of tokens. This type
 * of parameter must always be the last parameter of a command.
 *
 * Parameter type: CLI_PARAM_TYPE_VARARG
 */
int cli_params_add_vararg(SCliParams * pParams, char * pcName,
			  uint8_t uMaxArgs,
			  FCliCheckParam fCheckParam)
{
  SCliCmdParam * pParam= cli_cmd_param_create(pcName, fCheckParam);
  _cli_params_check_latest(pParams);
  pParam->tType= CLI_PARAM_TYPE_VARARG;
  pParam->uMaxArgs= uMaxArgs;
  return ptr_array_add((SPtrArray *) pParams, &pParam);
}


/////////////////////////////////////////////////////////////////////
//
// COMMANDS MANAGMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// ----- cli_cmd_create ---------------------------------------------
/**
 *
 */
SCliCmd * cli_cmd_create(char * pcName,
			 FCliCommand fCommand,
			 SCliCmds * pSubCmds,
			 SCliParams * pParams)
{
  SCliCmd * pCmd= (SCliCmd *) MALLOC(sizeof(SCliCmd));

  pCmd->pcName= (char *) MALLOC(sizeof(char)*(strlen(pcName)+1));
  strcpy(pCmd->pcName, pcName);
  pCmd->pParams= pParams;
  pCmd->pSubCmds= pSubCmds;
  pCmd->fCtxCreate= NULL;
  pCmd->fCtxDestroy= NULL;
  pCmd->fCommand= fCommand;
  pCmd->pcHelp= NULL;
  return pCmd;
}

// ----- cli_cmd_create_ctx -----------------------------------------
/**
 *
 */
SCliCmd * cli_cmd_create_ctx(char * pcName,
			     FCliContextCreate fCtxCreate,
			     FCliContextDestroy fCtxDestroy,
			     SCliCmds * pSubCmds,
			     SCliParams * pParams)
{
  SCliCmd * pCmd= (SCliCmd *) MALLOC(sizeof(SCliCmd));

  pCmd->pcName= (char *) MALLOC(sizeof(char)*(strlen(pcName)+1));
  strcpy(pCmd->pcName, pcName);
  pCmd->pParams= pParams;
  pCmd->pSubCmds= pSubCmds;
  pCmd->fCtxCreate= fCtxCreate;
  pCmd->fCtxDestroy= fCtxDestroy;
  pCmd->fCommand= NULL;
  pCmd->pcHelp= NULL;
  return pCmd;
}

// ----- cli_cmd_destroy --------------------------------------------
/**
 *
 */
void cli_cmd_destroy(SCliCmd ** ppCmd)
{
  if (*ppCmd != NULL) {
    FREE((*ppCmd)->pcName);
    ptr_array_destroy(&(*ppCmd)->pSubCmds);
    ptr_array_destroy(&(*ppCmd)->pParams);
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
  SCliCmdParam * pParam;
  char * pcNewPrefix;

  log_printf(pStream, "%s%s", pcPrefix, pCmd->pcName);
  if (pCmd->pParams != NULL)
    for (iIndex= 0; iIndex < ptr_array_length(pCmd->pParams); iIndex++) {
      pParam= (SCliCmdParam *) pCmd->pParams->data[iIndex];
      log_printf(pStream, " %s", pParam->pcName);
      if (pParam->tType == CLI_PARAM_TYPE_VARARG) {
	if (pParam->uMaxArgs > 0)
	  log_printf(pStream, "[0-%d]", pParam->uMaxArgs);
	else
	  log_printf(pStream, "[0-any]");
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

// ----- cli_cmds_item_compare --------------------------------------
/**
 * Compare two commands based on their names. The alphanumeric
 * ordering is used for the comparison.
 */
int cli_cmds_item_compare(void * pItem1, void * pItem2,
			  uint32_t EltSize)
{
  SCliCmd * pCmd1= *((SCliCmd **) pItem1);
  SCliCmd * pCmd2= *((SCliCmd **) pItem2);

  return strcmp(pCmd1->pcName, pCmd2->pcName);
}

// ----- cli_cmds_item_destroy --------------------------------------
/**
 * Free a command in a list of commands.
 */
void cli_cmds_item_destroy(void * pItem)
{
  cli_cmd_destroy((SCliCmd **) pItem);
}

// ----- cli_cmds_create --------------------------------------------
/**
 * Create a list of command. The commands in the list are sorted based
 * on the alphanumeric ordering (thanks to the cli_cmds_item_compare
 * function).
 */
SCliCmds * cli_cmds_create()
{
  return (SCliCmds *) ptr_array_create(ARRAY_OPTION_SORTED,
				       cli_cmds_item_compare,
				       cli_cmds_item_destroy);
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
				  cli_cmds_item_compare, NULL);

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


// ----- cli_cmds_add -----------------------------------------------
/**
 * Add a command (pCmd) in the list (pCmds).
 */
int cli_cmds_add(SCliCmds * pCmds, SCliCmd * pCmd)
{
  return ptr_array_add((SPtrArray *) pCmds, &pCmd);
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
  return cli_cmds_add(pCmd->pSubCmds, pSubCmd);
}

// ----- cli_cmd_add_param ------------------------------------------
/**
 *
 */
int cli_cmd_add_param(SCliCmd * pCmd, char * pcName,
		      FCliCheckParam fCheckParam)
{
  if (pCmd->pParams == NULL)
    pCmd->pParams= cli_params_create();
  return cli_params_add(pCmd->pParams, pcName, fCheckParam);
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
SCliCmdParam * cli_cmd_get_param_at(SCliCmd * pCmd, uint32_t uIndex)
{
  return (SCliCmdParam *) pCmd->pParams->data[uIndex];
}

// ----- cli_cmd_match_subcmds --------------------------------------
/**
 *
 */
SCliCmd * cli_cmd_match_subcmds(SCli * pCli, SCliCmd * pCmd,
				char * pcStartCmd,
				int * piParamIndex)
{
  STokens * pTokens;
  int iTokenIndex= 0;
  char * pcToken;
  SCliCmd * pSubCmd= NULL;

  if (tokenizer_run(pCli->pTokenizer, pcStartCmd) < 0) {
    return NULL;
  }

  pTokens= tokenizer_get_tokens(pCli->pTokenizer);

  // Match all tokens...
  while (iTokenIndex < tokens_get_num(pTokens)) {
    pcToken= tokens_get_string_at(pTokens, iTokenIndex);
    pSubCmd= cli_cmd_find_subcmd(pCmd, pcToken);
    if (pSubCmd == NULL)
      break;
    iTokenIndex++;
    pCmd= pSubCmd;
    *piParamIndex= 0;
    // Match params...
    while ((iTokenIndex < tokens_get_num(pTokens)) &&
	   (*piParamIndex < cli_cmd_get_num_params(pSubCmd))) {
      iTokenIndex++;
      (*piParamIndex)++;
    }
  }

  return pSubCmd;
}

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
  pCli->pExecCmd= NULL;
  pCli->pExecParam= NULL;
  pCli->pExecContext= cli_context_create();
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
    cli_context_destroy(&(*ppCli)->pExecContext);
    cli_cmd_destroy(&(*ppCli)->pBaseCommand);
    FREE(*ppCli);
    *ppCli= NULL;
  }
}

// ----- cli_execute_cmd --------------------------------------------
/**
 * This function executes a CLI command.
 *
 * The function modifies the following state variable of the CLI:
 *   - pExecParam
 *   - iExecTokenIndex
 *   - pExecCmd: CHECK THAT pCmd AND pCli->pExecCmd ARE THE SAME. IF
 *               SO, REMOVE ARGUMENT pCmd
 *   - pExecContext: will push context on the stack if the current
 *                   command produces context
 */
int cli_execute_cmd(SCli * pCli, SCliCmd * pCmd)
{
  int iParamIndex;
  int iResult;
  char * pcParamValue;
  STokens * pTokens= tokenizer_get_tokens(pCli->pTokenizer);
  void * pContextItem;
  int iNumTokenToEat;

  // --------------------------------------------
  // Step (1): Match all the command's parameters
  // --------------------------------------------
  iParamIndex= 0;
  while (iParamIndex < cli_cmd_get_num_params(pCli->pExecCmd)) {    
    pCli->pExecParam= cli_cmd_get_param_at(pCli->pExecCmd, iParamIndex);

    // If the parameter is of type vararg (CLI_PARAM_TYPE_VARARG), it
    // will eat all the remaining arguments. In the other case, we
    // must check if there is still enough parameters.
    if (pCli->pExecParam->tType == CLI_PARAM_TYPE_VARARG) {
      // Eat all the remaining arguments (0 args is allowed)
      iNumTokenToEat= tokens_get_num(pTokens) - pCli->iExecTokenIndex;
      if ((pCli->pExecParam->uMaxArgs > 0) &&
	  (iNumTokenToEat > pCli->pExecParam->uMaxArgs))
	return CLI_ERROR_TOO_MANY_PARAMETERS;
    } else {
      // Still enough parameters in command-line ?
      if (pCli->iExecTokenIndex >= tokens_get_num(pTokens))
	return CLI_ERROR_MISSING_PARAMETER;
      iNumTokenToEat= 1;
    }

    // Eat the parameters...
    while (iNumTokenToEat > 0) {
      pcParamValue= tokens_get_string_at(pTokens, pCli->iExecTokenIndex);
      // Check the parameter's value (if required)
      if (pCli->pExecParam->fCheckParam != NULL) {
	iResult= pCli->pExecParam->fCheckParam(pcParamValue);
	if (iResult)
	  return iResult; // parameter error !!!
      }
      cli_context_add_token_copy(pCli->pExecContext, pcParamValue);
      pCli->iExecTokenIndex++;
      iNumTokenToEat--;
    }
    iParamIndex++;
  }

  // --------------------------------------------
  // Step (2): Produce context ?
  // --------------------------------------------
  if (pCli->pExecCmd->fCtxCreate != NULL) {
    pContextItem= NULL;
    iResult= pCli->pExecCmd->fCtxCreate(pCli->pExecContext, &pContextItem);
    if (iResult)
      return iResult;
    cli_context_push(pCli->pExecContext, pContextItem, pCli->pExecCmd);
  }

  // --------------------------------------------
  // Step (3): Execute command ?
  // --------------------------------------------
  if (pCli->iExecTokenIndex == tokens_get_num(pTokens)) {
    if (pCli->pExecCmd->fCommand != NULL) {
      iResult= pCli->pExecCmd->fCommand(pCli->pExecContext,
					pCli->pExecContext->pTokens);
      // Return to saved context level
      cli_context_restore_depth(pCli->pExecContext);
      if (iResult)
	return iResult;
    } else
      if (pCli->pExecCmd->fCtxCreate == NULL)
	return CLI_ERROR_NOT_A_COMMAND;
  }

  return CLI_SUCCESS;
}

// ----- cli_execute_ctx --------------------------------------------
/**
 *
 */
int cli_execute_ctx(SCli * pCli, char * pcCmd, void * pContext)
{
  int iResult= CLI_SUCCESS;
  STokens * pTokens;
  char * pcToken;
  SCliCmd * pNewCmd;
  SCliCtxItem * pCtxItem;

  cli_context_set(pCli->pExecContext, pContext);

  // Split command line into tokens...
  if (tokenizer_run(pCli->pTokenizer, pcCmd))
    return CLI_ERROR_UNEXPECTED;
  pTokens= tokenizer_get_tokens(pCli->pTokenizer);

  // Empty line ?
  if (tokens_get_num(pTokens) == 0) {
    cli_context_clear(pCli->pExecContext);
    return CLI_SUCCESS;
  }
  // Single "exit" token ?
  if ((tokens_get_num(pTokens) == 1) &&
      (!strcmp("exit", tokens_get_string_at(pTokens, 0)))) {
    cli_context_backtrack(pCli->pExecContext);
    return CLI_SUCCESS;
  }
  // Ends with "?" token ?
  if (!strcmp("?", tokens_get_string_at(pTokens, tokens_get_num(pTokens)-1))) {
    return CLI_SUCCESS_HELP;
  }
  // Context available ?
  if (!cli_context_is_empty(pCli->pExecContext)) {
    pCtxItem= cli_context_top(pCli->pExecContext);
    assert(pCtxItem != NULL);
    pCli->pExecCmd= pCtxItem->pCmd;
    // Strip params ?
    while (tokens_get_num(pCli->pExecContext->pTokens) > pCtxItem->uNumParams)
      ptr_array_remove_at((SPtrArray *) pCli->pExecContext->pTokens,
			  tokens_get_num(pCli->pExecContext->pTokens)-1);
  } else {
    pCli->pExecCmd= pCli->pBaseCommand;
    tokens_destroy(&pCli->pExecContext->pTokens);
    pCli->pExecContext->pTokens= tokens_create();
  }
  pCli->iExecTokenIndex= 0;
  pCli->pExecParam= NULL;
  cli_context_save_depth(pCli->pExecContext);

  // Match all tokens...
  while (pCli->iExecTokenIndex < tokenizer_get_num_tokens(pCli->pTokenizer)) {
    if (pCli->pExecCmd != NULL) {
      pcToken= tokens_get_string_at(pTokens, pCli->iExecTokenIndex);
      pNewCmd= cli_cmd_find_subcmd(pCli->pExecCmd, pcToken);
      if (pNewCmd == NULL) {
	iResult= CLI_ERROR_UNKNOWN_COMMAND;
	break;
      }
      pCli->pExecCmd= pNewCmd;
      pCli->iExecTokenIndex++;
      iResult= cli_execute_cmd(pCli, pCli->pExecCmd);
      if (iResult)
	break;
    } else {
      iResult= CLI_ERROR_UNKNOWN_COMMAND;
      break;
    }
  }
  
  if (iResult < 0)
    cli_context_restore_depth(pCli->pExecContext);

  return iResult;
}

// ----- cli_execute ------------------------------------------------
/**
 *
 */
int cli_execute(SCli * pCli, char * pcCmd)
{
  return cli_execute_ctx(pCli, pcCmd, NULL);
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
  switch (iErrorCode) {
  case CLI_SUCCESS:
    log_printf(pStream, "success\n"); break;
  case CLI_ERROR_UNEXPECTED:
    log_printf(pStream, "unexpected error\n"); break;
  case CLI_ERROR_UNKNOWN_COMMAND:
    log_printf(pStream, "unknown command\n"); break;
  case CLI_ERROR_MISSING_PARAMETER:
    log_printf(pStream, "missing parameter\n"); break;
  case CLI_ERROR_TOO_MANY_PARAMETERS:
    log_printf(pStream, "too many parameters (vararg)\n"); break;
  case CLI_ERROR_NOT_A_COMMAND:
    log_printf(pStream, "not a command\n"); break;
  case CLI_ERROR_COMMAND_FAILED:
    log_printf(pStream, "command failed\n"); break;
  case CLI_ERROR_BAD_PARAMETER:
    log_printf(pStream, "bad parameter value\n"); break;
  case CLI_ERROR_CTX_CREATE:
    log_printf(pStream, "unable to create context\n"); break;
  case CLI_WARNING_EMPTY_COMMAND:
    log_printf(pStream, "empty command\n"); break;
  default:
    log_printf(pStream, "unknown error\n");
  }
}

// ----- cli_perror_details -----------------------------------------
/**
 *
 */
void cli_perror_details(SLogStream * pStream, int iResult, SCli * pCli,
			char * pcLine)
{
  STokens * pTokens;
  int iIndex;

  log_printf(pStream, "*** command: \"%s\"\n", pcLine);
  if ((iResult == CLI_ERROR_UNKNOWN_COMMAND) ||
      (iResult == CLI_ERROR_NOT_A_COMMAND) ||
      (iResult == CLI_ERROR_MISSING_PARAMETER) ||
      (iResult == CLI_ERROR_BAD_PARAMETER)) {
    log_printf(pStream, "*** error  : \"");
    pTokens= tokenizer_get_tokens(pCli->pTokenizer);
    for (iIndex= 0; iIndex < pCli->iExecTokenIndex; iIndex++)
      log_printf(pStream, "%s ", tokens_get_string_at(pTokens, iIndex));
    log_printf(pStream, "^^^\"\n");
    if (((iResult == CLI_ERROR_UNKNOWN_COMMAND) ||
	(iResult == CLI_ERROR_NOT_A_COMMAND)) &&
	(pCli->pExecCmd != NULL)) {
      log_printf(pStream, "*** expect : ");

      for (iIndex= 0;
	   iIndex < cli_cmd_get_num_subcmds(pCli->pExecCmd);
	   iIndex++) {
	if (iIndex > 0)
	  log_printf(pStream, ", ");
	log_printf(pStream, "%s",
		cli_cmd_get_subcmd_at(pCli->pExecCmd, iIndex)->pcName);
      }
      log_printf(pStream, "\n");
    } else if (((iResult == CLI_ERROR_MISSING_PARAMETER) ||
	 (iResult == CLI_ERROR_BAD_PARAMETER)) &&
	(pCli->pExecParam != NULL))
      log_printf(pStream, "*** expect : %s\n", pCli->pExecParam->pcName);
  }
}

// ----- cli_execute_line -------------------------------------------
/**
 *
 */
int cli_execute_line(SCli * pCli, char * pcLine)
{
  int iLen;
  int iResult= CLI_SUCCESS;

  // Skip commented lines
  if (pcLine[0] != '#') {
    // Chop '\n' at end of line
    iLen= strlen(pcLine);
    if ((iLen >= 1) && (pcLine[iLen-1] == '\n'))
      pcLine[iLen-1]= '\0';
    // Parse and execute command
    iResult= cli_execute(pCli, pcLine);
    if (iResult < 0) {
      log_printf(pLogErr, "\033[0;31;1mError: ");
      cli_perror(pLogErr, iResult);
      log_printf(pLogErr, "\033[0m");
      cli_perror_details(pLogErr, iResult, pCli, pcLine);
    }
  }
  return iResult;
}

// ----- cli_execute_file -------------------------------------------
/**
 * Execute all the lines from the given input stream.
 *
 * NOTE: The execution fo the script will stop at the first error.
 */
int cli_execute_file(SCli * pCli, FILE * pStream)
{
  char acLine[1024];
  int iResult;
  uint32_t uLineNumber= 1;

  /* Execute all lines in the input file... */
  while (fgets(acLine, sizeof(acLine), pStream) != NULL) {
    iResult= cli_execute_line(pCli, acLine);
    if (iResult < 0) {
      log_printf(pLogErr, "Error: in script file, line %u\n", uLineNumber);
      return iResult;
    }
    uLineNumber++;
  }

  /* Clear the context (if required) */
  cli_context_clear(pCli->pExecContext);

  return CLI_SUCCESS;
}
